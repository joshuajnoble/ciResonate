#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"

#include "CinderOpenCV.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class bgSubtractApp : public AppBasic {
  public:
	void setup();
	void keyDown(KeyEvent key);
	void update();
	void draw();
    
    cv::BackgroundSubtractorMOG bgsubtractor;
    vector< vector<cv::Point> > contours;
    
    vector<PolyLine2f> contourLine;
    
    bool updateBackground;
    Capture capture;
    Surface captureSurf;
    gl::Texture foregroundTexture, backgroundTexture;
    
    int drawMode;
    
};

void bgSubtractApp::setup()
{
    drawMode = 0;
    
    //bgsubtractor.noiseSigma = 10;
    updateBackground = true;
    
    
    gl::Texture::Format fmt;
    foregroundTexture = gl::Texture(640, 480, fmt);
    backgroundTexture = gl::Texture(640, 480, fmt);
    
    capture = Capture(640, 480);
    capture.start();
}

void bgSubtractApp::keyDown(KeyEvent key) {
    drawMode++;
    if(drawMode > 2) {
        drawMode = 0;
    }
}


void bgSubtractApp::update()
{
    
    if(capture.checkNewFrame()) {
        
        if(getElapsedFrames() % 120 == 0) { // only do this only every 4 secs or so
           updateBackground = true;
        }
        
        captureSurf = capture.getSurface();
        cv::Mat working, newFrame, fgFrame;
        newFrame = toOcv( captureSurf ); // capture the frame
        
        // case you're wondering, this is overloaded (), not a constructor call
        bgsubtractor(newFrame, fgFrame, updateBackground ? 1 : 0);
        
        updateBackground = false;
        
        int numberOfIters = 3;
        vector<cv::Vec4i> hierarchy;
        
        cv::dilate(fgFrame, working, cv::Mat(), cv::Point(-1,-1), numberOfIters);
        cv::erode(working, working, cv::Mat(), cv::Point(-1,-1), numberOfIters * 2);
        cv::dilate(working, working, cv::Mat(), cv::Point(-1,-1), numberOfIters);
        
        contours.clear();
        
        cv::findContours( working, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
        
        contourLine.clear();
        
        for( int i = 0; i < contours.size(); i++) {
            contourLine.push_back(PolyLine2f());
            for( int j = 0; j < contours[i].size(); j++) {
                contourLine.at(i).push_back(Vec2f(contours[i][j].x, contours[i][j].y) );
            }
        }
        
        if( contours.size() == 0 )
            return;
        
        // we're going to ^ the mask
        cv::Mat filledFG, filledBG;
        filledBG = cv::Mat::zeros(fgFrame.size(), CV_32F);
        filledFG = cv::Mat::zeros(fgFrame.size(), CV_32F);
        
        // get the foreground
        newFrame.copyTo(filledFG, fgFrame);
        // now get the background
        newFrame.copyTo(filledBG, 255 - fgFrame);
        
        backgroundTexture.update( Surface(fromOcv(filledBG) ));
        foregroundTexture.update( Surface(fromOcv(filledFG) ));

    }
}

void bgSubtractApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
    
    switch (drawMode) {
        case 0:
            if(backgroundTexture)
                gl::draw(backgroundTexture);
            break;
        case 1:
            if(foregroundTexture)
                gl::draw(foregroundTexture);
            break;
        case 2:
            for( int i = 0; i < contourLine.size(); i++) {
                gl::draw(contourLine.at(i));
            }
            break;
    }
}


CINDER_APP_BASIC( bgSubtractApp, RendererGl )
