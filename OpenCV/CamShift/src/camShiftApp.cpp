#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Capture.h"
#include "cinder/gl/Texture.h"
#include "CinderOpenCV.h"
#include "cinder/params/Params.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// ROBERT NOTE: Do we have to do it this way? The cammShiftApp() bit is confusing to the eye.
class camShiftApp : public AppNative {
public:
    cv::Scalar NOISE_STD;			// AWGN standard deviation that afflicts the measurement vector z_k
    cv::Scalar NOISE_MEAN;			// AWGN mean that afflicts the measurement vector z_k
    
    camShiftApp();
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
    bool inline boundCheck( cv::Rect rect, float vmin, float vmax ) {
        return rect.width > vmin && rect.width < vmax && rect.height > vmin && rect.width < vmax;
    };
    
    cv::KalmanFilter mKalman;
    cv::Rect mSelection;
    cv::Mat mHSVMask, mHistogram, mBackprojected;
    
    Capture mCapture;
    gl::Texture mCaptureTex, mHSVMaskTex;
    
    cv::Mat mProcessNoise;
    cv::Mat_<float> mKalmanMeasurement; /* (x, y, Vx, Vy) */
    
    cv::RotatedRect mTrackBox;
    
    float hranges[2];
    
    params::InterfaceGl params;
    int trackingHue, trackingSat, trackingValue, rangeWidth;
    
    int *channels;
    
};

camShiftApp::camShiftApp()
	: mKalman(4, 2, 0), mKalmanMeasurement(2,1, CV_32F), mProcessNoise(4, 1, CV_32F), NOISE_MEAN(0), NOISE_STD(2)
{

    mKalmanMeasurement.setTo(cv::Scalar(0));
    
    hranges[0] = 0.f; hranges[1] = 180.f;
    
    channels = new int[2];
    channels[0] = 0;
    channels[1] = 1;
    
    trackingHue = 63;
    trackingSat = 230;
    trackingValue = 230;
    
    rangeWidth = 100;
}

void camShiftApp::setup()
{
	
    mSelection.x = 0;
    mSelection.y = 0;
    mSelection.width = 0;
    mSelection.height = 0;
    
    mKalmanMeasurement.setTo(cv::Scalar(0));
    
	mKalman.transitionMatrix = *(cv::Mat_<float>(4, 4) << 
                                1,0,1,0, // x + dx 
                                0,1,0,1, // y + dy
                                0,0,1,0, // dx
                                0,0,0,1); // dy
    
	// init...
	mKalman.statePre.at<float>(0) = 0;
	mKalman.statePre.at<float>(1) = 0;
	mKalman.statePre.at<float>(2) = 0;
	mKalman.statePre.at<float>(3) = 0;
    
	cv::setIdentity(mKalman.measurementMatrix);
	cv::setIdentity(mKalman.processNoiseCov, cv::Scalar::all(1e-4));
	cv::setIdentity(mKalman.measurementNoiseCov, cv::Scalar::all(1e-1));
	cv::setIdentity(mKalman.errorCovPost, cv::Scalar::all(.1));
    
    mCaptureTex = gl::Texture(640, 480, gl::Texture::Format());
    mHSVMaskTex = gl::Texture(640, 480, gl::Texture::Format()); 
    
    // Setup the parameters
	params = params::InterfaceGl( "params", Vec2i( 200, 200 ) );
	params.addParam( "Tracking Hue", &trackingHue);
    params.addParam( "Tracking Sat", &trackingSat);
    params.addParam( "Tracking Value", &trackingValue);
    params.addParam( "Tracking Width", &rangeWidth);
    
    mCapture = Capture(640, 480);
	mCapture.start();
    
    gl::enableAlphaBlending();
    
}

void camShiftApp::mouseDown( MouseEvent event )
{
    
    // draw a box around what we want to grab
    mSelection.x = max(event.getX() - 50, 0);
    mSelection.y = max(event.getY() - 50, 0);
    mSelection.width = min(50, getWindowWidth() - event.getX());
    mSelection.height = min(50, getWindowHeight() - event.getY());
	
    cv::Mat frame = toOcv( mCapture.getSurface() );
    cv::Mat hsv;
    cvtColor(frame, hsv, CV_BGR2HSV);
    
    cv::inRange(hsv, 
                cv::Scalar(trackingHue - rangeWidth, trackingSat - rangeWidth, trackingValue - rangeWidth), 
                cv::Scalar(trackingHue + rangeWidth, trackingSat + rangeWidth, trackingValue + rangeWidth), 
                mHSVMask);
    
    const float* pointerToRanges = &hranges[0];
    
    int histSize = 16;
    cv::calcHist(&frame, 1, &channels[0], mHSVMask, mHistogram, 1, &histSize, &pointerToRanges);
    cv::normalize(mHistogram, mHistogram, 0, 255, CV_MINMAX);
    
}

void camShiftApp::update()
{
    //
	if(!mCapture.checkNewFrame()) {
		return; // nothing to do
	}
    
    mCaptureTex.update(mCapture.getSurface());

    // make an HSV frame
    cv::Mat newFrameHSV;
    cv::Mat newFrame = toOcv( mCapture.getSurface() );    
    cv::cvtColor(newFrame, newFrameHSV, CV_BGR2HSV);
    
    // find the parts of the HSV frame that have our requested range
    cv::inRange(newFrameHSV, 
                cv::Scalar(trackingHue - rangeWidth, trackingSat - rangeWidth, trackingValue - rangeWidth), 
                cv::Scalar(trackingHue + rangeWidth, trackingSat + rangeWidth, trackingValue + rangeWidth), 
                mHSVMask);
    
	mHSVMaskTex = gl::Texture(fromOcv(mHSVMask));
    
    mBackprojected = cv::Mat::zeros(newFrameHSV.rows, newFrameHSV.cols, CV_8UC3);
    
    // if we haven't set hist, there's nothing to track done
    if(mHistogram.total() < 1) {
        return;
    }
    
    // do back projection 
    const float* annoyingPointerRequiredByBackProject = &hranges[0];
    cv::calcBackProject(&newFrameHSV, 1, channels, mHistogram, mBackprojected, &annoyingPointerRequiredByBackProject);
    mBackprojected &= mHSVMask;
    
    // now camshift
    mTrackBox = cv::CamShift(mBackprojected, mSelection, cv::TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));
    
    // First predict, to update the internal statePre variable
    cv::Mat prediction = mKalman.predict();
    cv::Point predictPt(prediction.at<float>(0),prediction.at<float>(1));
	
    // if we got a mTrackBox back, update the window we're going to use as an initial location
    // otherwise, use the kalman filter prediction
    if(  boundCheck(mTrackBox.boundingRect(), 8, 1000) && mTrackBox.center.x < getWindowWidth() && mTrackBox.center.y < getWindowHeight() ) {
        
        console() << " mTrackBox is meaningful " << endl;
        
        // put the tracked rect into the measurement
        mKalmanMeasurement(0) = mTrackBox.center.x;
        mKalmanMeasurement(1) = mTrackBox.center.y;
        
        // The "correct" phase that is going to use the predicted value and our measurement
        mKalman.correct(mKalmanMeasurement);
        
        mSelection = mTrackBox.boundingRect();
        
    } else {
        
        console() << " mTrackBox is NOT meaningful " << endl;
        cv::Mat sum, pre = mKalman.statePre, rand = cv::Mat(4, 1, CV_32F);
        
        cv::randn(rand,NOISE_MEAN, NOISE_STD);
        cv::add(pre,rand,sum);
        
        mKalmanMeasurement(0) = sum.at<float>(0,0);
        mKalmanMeasurement(1) = sum.at<float>(1,0);
        
        // The "correct" phase that is going to use the predicted value and our measurement
        mKalman.correct(mKalmanMeasurement);
        
        // update our selection
        mSelection.x = predictPt.x;
        mSelection.y = predictPt.y;
        mSelection.width = 50;
        mSelection.height = 50;
    }
}

void camShiftApp::draw()
{
    // clear out the window with black
	gl::clear( Color( 0, 0, 0  ) ); 
    
    gl::color(1, 1, 1, 1);
    if(mCaptureTex) {
        gl::draw(mCaptureTex);
    }
    
    gl::color(1, 1, 1, 0.5);
    if(mHSVMaskTex) {
        gl::draw(mHSVMaskTex);
    }
	
    gl::color(1, 0, 0, 1.f);
    cv::Point2f pts[4];
    mTrackBox.points(pts);
    
    gl::drawLine( fromOcv(pts[0]), fromOcv(pts[1]));
    gl::drawLine( fromOcv(pts[1]), fromOcv(pts[2]));
    gl::drawLine( fromOcv(pts[2]), fromOcv(pts[3]));
    gl::drawLine( fromOcv(pts[3]), fromOcv(pts[0]));
	
    // Draw the interface
    params.draw();
}

CINDER_APP_NATIVE( camShiftApp, RendererGl )
