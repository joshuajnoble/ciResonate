#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Capture.h"
#include "cinder/ip/Resize.h"

#include "CinderOpenCV.h"

#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class peopleDetectApp : public AppBasic {
public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
    Capture capture;
    
    cv::HOGDescriptor hog;
    cv::Mat img;
    vector<cv::Rect> foundLocations;
    
    vector<Rectf> ciPeople;
    
    Surface stillSurface;
    gl::Texture stillTex;
};

void peopleDetectApp::setup()
{
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
    
    capture = Capture(640, 480);
    capture.start();
    
    stillSurface = loadImage(loadAsset( "people_in_park.jpg" ) );
	stillTex = gl::Texture(stillSurface);
    
}

void peopleDetectApp::mouseDown( MouseEvent event )
{
}

void peopleDetectApp::update()
{
    
    //if(capture.checkNewFrame()) {
    if(stillSurface) {
        //cv::Mat newFrame = toOcv( Channel( capture.getSurface() )); // capture the frame
        cv::Mat newFrame = toOcv( Channel(stillSurface));
        
        vector<cv::Rect> found, found_filtered;
        // run the detector with default parameters. to get a higher hit-rate
        // (and more false alarms, respectively), decrease the hitThreshold and
        // groupThreshold (set groupThreshold to 0 to turn off the grouping completely).
        //hog.detectMultiScale(newFrame, foundLocations, 0, cv::Size(8,8), cv::Size(32,32), 1.05, 2);
        hog.detectMultiScale(newFrame, foundLocations, 0, cv::Size(8,8), cv::Size(32,32), 1.05, 2);
        
        int i,j;
        for( i = 0; i < foundLocations.size(); i++ )
        {
            cv::Rect r = foundLocations[i];
            for( j = 0; j < foundLocations.size(); j++ ) {
                if( j != i && (r & foundLocations[j]) == r)
                    break;
                if( j == foundLocations.size() )
                    found_filtered.push_back(r);
            }
        }
        
        console() << " found # " << foundLocations.size() << " filtered to " << found_filtered.size() << endl;
        
        ciPeople.clear();
        for( i = 0; i < foundLocations.size(); i++ ) {
            
            cv::Rect r = foundLocations[i];
            // the HOG detector returns slightly larger rectangles than the real objects.
            // so we slightly shrink the rectangles to get a nicer output.
            r.x += cvRound(r.width*0.1);
            r.width = cvRound(r.width*0.8);
            r.y += cvRound(r.height*0.07);
            r.height = cvRound(r.height*0.8);
            ciPeople.push_back(fromOcv(r));
        }
    }
}

void peopleDetectApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
    
    if(stillTex) 
        gl::draw(stillTex); // CHANGETHIS
    
    for(int i = 0; i < ciPeople.size(); i++) {
        gl::drawStrokedRect(ciPeople[i]);
    }
    
}


CINDER_APP_NATIVE( peopleDetectApp, RendererGl )
