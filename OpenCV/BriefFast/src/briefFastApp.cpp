#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Capture.h"
#include "cinder/gl/Texture.h"
#include "Resources.h"

#include "CinderOpenCV.h"


using namespace ci;
using namespace ci::app;
using namespace std;

class briefFastApp : public AppBasic {
    
    public:
    
    briefFastApp();
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
    gl::Texture texture, objectTexture;
	ci::Surface surf;
	ci::Capture capture;
	
    cv::BFMatcher matcher;
    
    cv::FastFeatureDetector detector;
    cv::BriefDescriptorExtractor extractor;
    
    vector<cv::KeyPoint> patternPoints, scenePoints;
    cv::Mat pattern, scene, patternDescriptor, sceneDescriptor;
    vector<cv::DMatch> matches;
    gl::Texture patternTex, sceneTex;
};

//this is really 32 x 8 matches since they are binary matches packed into bytes
briefFastApp::briefFastApp() : detector(50), extractor(32), matcher(cv::NORM_HAMMING) {
    
}

void briefFastApp::setup() {
    
    capture = Capture(320, 240);
    capture.start();
    
    patternTex = gl::Texture(320, 240, gl::Texture::Format());
    sceneTex = gl::Texture(320, 240, gl::Texture::Format());
    
    // this works best if you have a can of Goya Black Beans laying around :)
    // of course, any object can be located and tracked.
    Surface imageTemplate = Surface(loadImage(loadAsset("iphone.jpg")));
    
    patternTex = gl::Texture( imageTemplate );
    pattern = toOcv(Channel( imageTemplate ));
    
    detector.detect(pattern, patternPoints);
    extractor.compute(pattern, patternPoints, patternDescriptor);
    
}

void briefFastApp::mouseDown( MouseEvent event ) {
    
}

void briefFastApp::update() {
    
	if(capture.checkNewFrame()) {
		surf = capture.getSurface();
	} else {
        return; // nothing to do
    }
    
    scene = toOcv( capture.getSurface() );
    sceneTex.update( capture.getSurface() );
    
    detector.detect(scene, scenePoints);
    extractor.compute(scene, scenePoints, sceneDescriptor);
    
    // Using features2d
    matcher.match(patternDescriptor, sceneDescriptor, matches);
    
}

void briefFastApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
    
    if(patternTex) {
        
        gl::draw(patternTex);
        for( int i = 0; i < matches.size(); i++) {
            gl::drawStrokedCircle(fromOcv(patternPoints[matches.at(i).trainIdx].pt), 5);
        }
        
        gl::pushMatrices();
        gl::translate(320, 0);
        gl::draw(sceneTex);
        
        for( int i = 0; i < matches.size(); i++)
        {
            if(scenePoints[matches.at(i).queryIdx].response > 10.0) {
                gl::drawStrokedCircle(fromOcv(scenePoints[matches.at(i).queryIdx].pt), 5);
            }
        }
        
        gl::popMatrices();
    }
}


CINDER_APP_BASIC( briefFastApp, RendererGl )
