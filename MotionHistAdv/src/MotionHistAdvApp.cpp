#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"
#include "CinderOpenCV.h"

#define MHI_DURATION 2.0//1.0
#define MAX_TIME_DELTA 1.0//0.5
#define MIN_TIME_DELTA 0.05

using namespace ci;
using namespace ci::app;
using namespace std;

static const int APP_WIDTH	= 640;
static const int APP_HEIGHT	= 480;

struct Quad {
	Quad(){}
	Quad( Vec2f p, Vec2f v, Surface s, Rectf r ) : mPos(p), mVel(v), mSurf(s), mRect(r)
	{
		mVel		*= Rand::randFloat( 10.0f );
		mAge		= 0.0f;
		mLifespan	= 150.0f;
		mAgePer		= 1.0f;
		
		mTex		= gl::Texture( mSurf );
	}
	void update()
	{
		mPos	+= mVel;
		mVel	*= 0.825f;
		
		mAge	+= 1.0f;
		mAgePer	 = 1.0f - mAge/mLifespan;
	}
	
	void draw()
	{
		Rectf r = mRect * mAgePer + mPos;
		gl::color( ColorA( 1, 1, 1, 1 ) );
		gl::draw( mTex, r );
		
		gl::color( ColorA( 0, 0, 0, 0.25f ) );
		gl::drawStrokedRect( r );
	}
	
	bool isDead(){
		return mAge > mLifespan;
	}
	
	Surface		mSurf;
	gl::Texture mTex;
	Vec2f		mPos;
	Vec2f		mVel;
	Rectf		mRect;
	float		mAge, mAgePer, mLifespan;
};


class motionHistAdvApp : public AppBasic {
public:
	void setup();
	void update();
	void draw();
    
	// CAPTURE
	Capture			mCapture;
	Surface			mCaptureSurface;
	gl::Texture		mCaptureTex;
	
	// VISUALS
	vector<Quad>	mQuads;
	
	// OPENCV
    cv::Mat mCurrentFrame, mPrevFrame;
	cv::Mat mInput, mMask, mMotionMask;
    cv::Mat mOrientation, mHistory;
    
    vector<cv::Rect>	mBoundingRects;
    vector<double>		mAngles;
    double				mMagnitude;
    
	// TEXTURES
    gl::Texture			mHistoryTex, mDiffTex, mOrientTex, mMotionMaskTex;
};

void motionHistAdvApp::setup()
{
	// CAPTURE
	mCaptureTex		= gl::Texture( APP_WIDTH, APP_HEIGHT );
	mCaptureSurface = Surface( APP_WIDTH, APP_HEIGHT, false );
    mCapture		= Capture( APP_WIDTH, APP_HEIGHT );
    mCapture.start();
    
	// OPENCV
	// NOTE THE WIDTH AND HEIGTH NEED TO BE REVERSED FOR THESE MATS
    mHistory		= cv::Mat::zeros( APP_HEIGHT, APP_WIDTH, CV_32FC1 );
    mMotionMask		= cv::Mat::zeros( APP_HEIGHT, APP_WIDTH, CV_32FC1 );
    mOrientation	= cv::Mat::zeros( APP_HEIGHT, APP_WIDTH, CV_32FC1 );
    mMask			= cv::Mat::zeros( APP_HEIGHT, APP_WIDTH, CV_8UC1 );
    
	// TEXTURES
    mHistoryTex		= gl::Texture( APP_WIDTH, APP_HEIGHT, gl::Texture::Format() );
    mDiffTex		= gl::Texture( APP_WIDTH, APP_HEIGHT, gl::Texture::Format() );
    mOrientTex		= gl::Texture( APP_WIDTH, APP_HEIGHT, gl::Texture::Format() );
    mMotionMaskTex	= gl::Texture( APP_WIDTH, APP_HEIGHT, gl::Texture::Format() );
    
    mMagnitude		= 50;
}

void motionHistAdvApp::update()
{
    if( !mCapture.checkNewFrame() ) {
		return; // nothing to do
	}
	
	float secs = getElapsedSeconds(); // always make sure that all the methods have the exact same time.
	mBoundingRects.clear();
    mAngles.clear();
    
	mCaptureSurface = mCapture.getSurface();
	mCaptureTex		= gl::Texture( mCaptureSurface );
	
    mPrevFrame		= mCurrentFrame;
    mCurrentFrame	= toOcv( Channel( mCaptureSurface ) );
    
    if( mPrevFrame.size().width == 0) {
        return;
    }
    
    cv::absdiff( mCurrentFrame, mPrevFrame, mInput );
    cv::threshold( mInput, mInput, 20, 1, cv::THRESH_BINARY );
    
    //void cv::updateMotionHistory( InputArray _silhouette, InputOutputArray _mhi, double timestamp, double duration )
    cv::updateMotionHistory( mInput, mHistory, secs, MHI_DURATION );
    
    cv::convertScaleAbs( mHistory, mMask, 255./MHI_DURATION, (MHI_DURATION - secs)*255.0/MHI_DURATION );
    
    // find the motion gradient
    cv::calcMotionGradient( mHistory, mMask, mOrientation, MAX_TIME_DELTA, MIN_TIME_DELTA, 3 );
	
    //find the motion segment
    vector<cv::Rect> tmpRects;
    cv::segmentMotion( mHistory, mMotionMask, tmpRects, secs, MIN_TIME_DELTA );
    
    if( tmpRects.size() == 0) {   
        mAngles.push_back( cv::calcGlobalOrientation( mOrientation, mMask, mHistory, secs, MHI_DURATION ) );
		
    } else {
        for( int i=0; i<tmpRects.size(); i++ ){
            if( tmpRects[i].area() > 200 ){
                cv::Mat mroi( mMask, tmpRects[i] );
                cv::Mat oroi( mOrientation, tmpRects[i] );
                cv::Mat hroi( mHistory, tmpRects[i] );
                double a = cv::calcGlobalOrientation( oroi, mroi, hroi, secs, MAX_TIME_DELTA );
                mAngles.push_back(a);
                mBoundingRects.push_back(tmpRects[i] );
				
				Rectf r = fromOcv( tmpRects[i] );
				Vec2f pos = r.getCenter();
				
				float xp = cos( toRadians( mAngles[i] ) );
				float yp = sin( toRadians( mAngles[i] ) );
				Vec2f vel = Vec2f( xp, yp );
				Area area = Area( r );
				area.expand( area.getWidth()/3, area.getHeight()/3 );
				Rectf rect = r - pos;
				Surface surf = mCaptureSurface.clone( area );
				mQuads.push_back( Quad( pos, vel, surf, rect ) );
            }
        }
    }
    
    mHistoryTex.update( Surface( fromOcv( mHistory ) ) );
    mMotionMaskTex.update( Surface( fromOcv( mMotionMask ) ) );
    mDiffTex.update( Surface( fromOcv( mInput ) ) );
    mOrientTex.update( Surface( fromOcv( mOrientation ) ) );
    
	for( vector<Quad>::iterator it = mQuads.begin(); it != mQuads.end(); ){
		if( it->isDead() ){
			it = mQuads.erase( it );
		} else {
			it->update();
			++it;
		}
	}
}

void motionHistAdvApp::draw()
{    
	gl::clear( Color( 0, 0, 0 ) ); 
    gl::color( 1, 1, 1 );
	gl::enableAlphaBlending();
	
	for( vector<Quad>::iterator it = mQuads.begin(); it != mQuads.end(); ++it ){
		it->draw();
	}
}


CINDER_APP_BASIC( motionHistAdvApp, RendererGl )
