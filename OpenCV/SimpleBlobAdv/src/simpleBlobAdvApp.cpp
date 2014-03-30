#include "cinder/app/AppNative.h"
#include "cinder/Capture.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/gl.h"
#include "cinder/PolyLine.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"

#include "CinderOpenCV.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static const int APP_WIDTH	= 640;
static const int APP_HEIGHT	= 480;

struct Blob {
	Blob( Vec2f p, float r ) : pos(p), rad(r) {
		age			= 0.0f;
		lifespan	= 5.0f;
		isDead		= false;
	}
	
	void update(){
		age	+= 1.0f;
		agePer = 1.0f - age/lifespan;
		
		if( age > lifespan ) isDead = true;
	}
	
	void draw(){
		//		gl::drawSolidCircle( pos, r );//rad * 0.8f );
		
		Rectf rect = Rectf( pos.x - rad, pos.y - rad, pos.x + rad, pos.y + rad );
		gl::drawSolidRect( rect );
	}
	
	Vec2f pos;
	float rad;
	float age;
	float lifespan;
	float agePer;
	bool isDead;
};

class simpleBlobAdvApp : public AppNative {
  public:
	void setup();
	void update();
	void draw();
	
	// CAPTURE
    Capture			mCapture;
    gl::Texture		mCaptureTex;
	Surface			mCaptureSurface;
	
	// TEXTURES
	gl::Texture		mBlobTex;
    
	// OPENCV
    cv::Ptr<cv::SimpleBlobDetector> mBlobDetector;
    vector<cv::KeyPoint>	mKeypoints;
    vector<cv::Point>		mApproxContours;
	
	// VISUALS
	vector<Blob>	mBlobs;
	vector<Vec2f>	mAnchors;
};

void simpleBlobAdvApp::setup()
{
	// CAPTURE
	mCaptureTex		= gl::Texture( APP_WIDTH, APP_HEIGHT );
	mCaptureSurface = Surface( APP_WIDTH, APP_HEIGHT, false );
    mCapture		= Capture( APP_WIDTH, APP_HEIGHT );
    mCapture.start();
	
	// TEXTURES
	mBlobTex		= gl::Texture( loadImage( loadAsset( "dot.png" ) ) );
    
    cv::SimpleBlobDetector::Params params;
    params.minThreshold		= 20;
    params.maxThreshold		= 90;
    params.thresholdStep	= 2;
    params.minArea			= 10; 
	params.maxArea			= 1000;
    params.minConvexity		= 0.3;
    params.maxConvexity		= 10;
    params.minInertiaRatio	= 0.01;
    params.filterByColor	= false;
    params.filterByCircularity = false;
    
    mBlobDetector = cv::Ptr<cv::SimpleBlobDetector>( new cv::SimpleBlobDetector( params ) );
}

void simpleBlobAdvApp::update()
{
	if( !mCapture.checkNewFrame() ){
		return; // nothing to do
	}
	
	mCaptureSurface = mCapture.getSurface();
	mCaptureTex.update( mCaptureSurface );
	
	mBlobDetector->detect( toOcv( mCaptureSurface ), mKeypoints );
	
	if( mKeypoints.size() < 1 )
		return;
	
	mBlobs.clear();
	for( int i=0; i<mKeypoints.size(); ++i )
	{
		Vec2i pos	= fromOcv( mKeypoints[i].pt );
		float rad	= 2.0f;
		
		ColorA col	= mCaptureSurface.getPixel( pos );
		mBlobs.push_back( Blob( pos, rad ) );
	}
	
	mAnchors.clear();
	for( vector<Blob>::iterator it1 = mBlobs.begin(); it1 != mBlobs.end(); ++it1 ){
		Vec2f pos1 = it1->pos;
		
		float closestDist		= 1000000.0f;
		Vec2f closestPos		= Vec2f::zero();
		Vec2f secondClosestPos	= Vec2f::zero();
		
		for( vector<Blob>::iterator it2 = mBlobs.begin(); it2 != mBlobs.end(); ++it2 ){
			if( it2 != it1 ){
				Vec2f pos2 = it2->pos;
				
				Vec2f dir = pos2 - pos1;
				float distSqrd = dir.lengthSquared();
				// IF BLOBS ARE CLOSER THAN THE CLOSESTDIST, SET NEW CLOSESTDIST
				if( distSqrd < closestDist ){
					closestDist = distSqrd;
					secondClosestPos = closestPos;
					closestPos = pos2;
				}
				
				// IF BLOBS ARE SUPER CLOSE, JOIN THEM WITH AN ANCHOR LINE
				if( distSqrd < 500.0f ){
					mAnchors.push_back( pos1 );
					mAnchors.push_back( pos2 );
				}
			}
			
		}
		
		// IF THE BLOBS ARE CLOSE ENOUGH TO BE CONNECTED...
		if( closestDist < 10000.0f ){
			// CONNECT THE BLOB TO ITS CLOSEST AND SECOND CLOSEST NEIGHBOR BLOBS
			mAnchors.push_back( pos1 );
			mAnchors.push_back( closestPos );
			
			if( secondClosestPos != Vec2f::zero() ){
				mAnchors.push_back( pos1 );
				mAnchors.push_back( secondClosestPos );
			}
		}
		
	}
}

void simpleBlobAdvApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::enableAlphaBlending();
    
	// MAKE MIRROR IMAGE
	gl::pushMatrices();
	gl::translate( getWindowCenter() );
	gl::scale( Vec2f( -1.0f, 1.0f ) );
	gl::translate( -getWindowCenter() );
	
	// DRAW WEBCAM IMAGE
	gl::color( Color( 1, 1, 1 ) );
    gl::draw( mCaptureTex );
	
	glBegin( GL_LINES );
	
	// DRAW LIGHT ANCHOR LINES
	gl::color( ColorA( 1, 1, 1, 0.1f ) );
	for( int i=0; i<mAnchors.size(); ++i ){
		gl::vertex( mAnchors[i] + Vec2f( 0.0f, -1.0f ) );
	}
	
	// DRAW DARK ANCHOR LINES
	gl::color( ColorA( 0, 0, 0, 0.2f ) );
	for( int i=0; i<mAnchors.size(); ++i ){
		gl::vertex( mAnchors[i] );
	}
	
	glEnd();
	
	gl::enable( GL_TEXTURE_2D );
	
	// DRAW BLOBS
	gl::color( Color( 1, 1, 1 ) );
	mBlobTex.bind();
	for( int i=0; i<mBlobs.size(); ++i ){
		mBlobs[i].draw();
	}
	
	gl::disable( GL_TEXTURE_2D );
	
	gl::popMatrices();
}

CINDER_APP_NATIVE( simpleBlobAdvApp, RendererGl )
