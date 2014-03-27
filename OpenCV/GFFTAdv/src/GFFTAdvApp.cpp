#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "CinderOpenCV.h"
#include "cinder/Capture.h"
#include "cinder/CinderMath.h"
#include "cinder/Rand.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace std;

struct Particle {
	Particle( Vec2f p, Vec2f v, ColorA c, bool b )
	: pos(p), vel(v), hasTail(b)
	{
		col			= Color( c.r, c.g, c.b ) * 1.1f;
		alpha		= c.a;
		age			= 0.0f;
		lifespan	= 20.0f;
		if( hasTail ) lifespan = 50.0f;
		isDead		= false;
	}
	
	void update(){
		oldPos = pos;
		pos += vel;
		vel *= 0.995f;
		vel += Vec2f( 0.0f, 0.125f );
		
		age	+= 1.0f;
		agePer = 1.0f - age/lifespan;
		if( age > lifespan ) isDead = true;
	}
	
	void draw(){
		float a = Rand::randFloat() * 0.3f;
		float b = Rand::randFloat( -0.05f, 0.05f );
		gl::color( col.r + b, col.g + b, col.b + b, agePer * 0.5f + a );
		gl::vertex( pos );
	}
	
	void drawLine(){
		gl::color( ColorA( col, alpha * agePer ) );
		gl::vertex( pos );
		gl::color( ColorA( col, 0.0f ) );
		gl::vertex( oldPos );
	}
	
	Vec2f pos, oldPos, vel;
	Color col;
	float alpha;
	float age, agePer, lifespan;
	bool hasTail;
	bool isDead;
};

static const int MAX_FEATURES			= 200;
static const int APP_WIDTH				= 640;
static const int APP_HEIGHT				= 480;
static const int CAM_WIDTH				= 320;
static const int CAM_HEIGHT				= 240;
static const float MIN_FEATURE_SPACING	= 3.0f;


class GFFTAdvApp : public AppBasic {
public:
	void prepareSettings( Settings *settings );
	void setup();
	void keyDown( KeyEvent event );
	void update();
	void draw();
    void calcField( );
    void drawField( float x, float y );
	
	// CAPTURE
	Capture					mCapture;
	gl::Texture				mCaptureTex;
	Surface					mCaptureSurface;
	
	// OPENCV
	cv::Mat					mPrevFrame, mCurrentFrame;
	vector<cv::Point2f>		mPrevFeatures, mFeatures;
	vector<uint8_t>			mFeatureStatuses;
	vector< pair<Vec2f,Vec2f> > mFlowPoints;
	
	Vec2f					mGlobalDir;
	float					mGlobalDirLength;
    
	// VISUALS
	bool					mDrawOpenCvData;
	vector<Particle>		mSegments;
    vector<Particle>		mSmallParticles;
    vector<Particle>		mLargeParticles;
	vector<Vec2f>			mAnchors;
	
    
};

void GFFTAdvApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( APP_WIDTH, APP_HEIGHT );
	settings->setFullScreen( false );
}

void GFFTAdvApp::setup()
{	
	// CAPTURE
	mCaptureSurface = Surface( CAM_WIDTH, CAM_HEIGHT, false );
	mCaptureTex		= gl::Texture( CAM_WIDTH, CAM_HEIGHT );
	mCapture		= Capture( CAM_WIDTH, CAM_HEIGHT );
	mCapture.start();
	
	mGlobalDir		= Vec2f::zero();
	
	mDrawOpenCvData	= false;
}

void GFFTAdvApp::keyDown( KeyEvent event )
{
	if( event.getChar() == ' ' ){
		mDrawOpenCvData = !mDrawOpenCvData;
	}
}

void GFFTAdvApp::calcField()
{
    mFlowPoints.clear();
	//    mFlowPoints.resize( MAX_FEATURES );
	
	for( int i=0; i<mFeatures.size(); i++ ){
		if( mFeatureStatuses[i] == 0 ){
			continue;
		}
		
		Vec2f prev( mPrevFeatures[i].x, mPrevFeatures[i].y );
		Vec2f curr( mFeatures[i].x, mFeatures[i].y );
		Vec2f vel = curr - prev;
		if( vel.lengthSquared() < 40000.0f )
			mFlowPoints.push_back( pair<Vec2f, Vec2f>( prev, curr ) );
	}
}

void GFFTAdvApp::update()
{
	if( mCapture.checkNewFrame() ) {
		mCaptureSurface = mCapture.getSurface();
		mCaptureTex		= gl::Texture( mCaptureSurface );
		mCurrentFrame	= toOcv( Channel( mCaptureSurface ) ) ;
		
		if( mPrevFrame.data ){
			// pick new features once every 30 frames, or the first frame
            if( mFeatures.empty() || getElapsedFrames() % 30 == 0 ){
				if( mGlobalDirLength < 0.5f ){
					cv::goodFeaturesToTrack( mCurrentFrame, mFeatures, MAX_FEATURES, 0.005, MIN_FEATURE_SPACING );
				}
            }
            
			vector<float> errors;
            mPrevFeatures = mFeatures;
			if( mFeatures.size() > 0 ){
				cv::calcOpticalFlowPyrLK( mPrevFrame, mCurrentFrame, mPrevFeatures, mFeatures, mFeatureStatuses, errors );
				calcField();
			}
		}
		mPrevFrame = mCurrentFrame;
		
		
		
		if( mFlowPoints.size() > 0 ){
			Vec2f vel;
			Vec2f avgVel = Vec2f::zero();
			for( int i=0; i<mFlowPoints.size(); ++i ){
				Vec2f old( mFlowPoints[i].first );
				Vec2f pos( mFlowPoints[i].second );
				vel = pos - old;
				avgVel += vel;
				
				ColorA col = mCaptureSurface.getPixel( Vec2i( pos ) );
				col.b += math<float>::min( mGlobalDirLength * 0.2f, 0.5f );
				
				if( pos.x > 0.0f ){
					mSegments.push_back( Particle( pos, vel * 0.5f, ColorA( 0.2f, 0.5f, 1.0f, math<float>::min( mGlobalDirLength * 0.1f, 0.1f ) ), true ) );
					
					mSmallParticles.push_back( Particle( pos, ( vel + Rand::randVec2f() ) * Rand::randFloat( 0.5f, 1.0f ), col, false ) );
					mLargeParticles.push_back( Particle( pos, ( vel + Rand::randVec2f() ) * Rand::randFloat( 0.25f, 0.5f ), col, false ) );
				}
			}
			avgVel /= mFlowPoints.size();
			
			Vec2f dir = avgVel;
			dir -= mGlobalDir;
			mGlobalDir += dir * 0.166;
			
			
			mAnchors.clear();
			for( vector<pair<Vec2f,Vec2f> >::iterator it1 = mFlowPoints.begin(); it1 != mFlowPoints.end(); ++it1 ){
				Vec2f pos1 = it1->first;
				
				float closestDist = 1000000.0f;
				Vec2f closestPos = Vec2f::zero();
				Vec2f secondClosestPos = Vec2f::zero();
				
				//			vector<Blob>::iterator it2 = it1;
				//			for( std::advance( it2, 1 ); it2 != blobs.end(); ++it2 ){
				for( vector<pair<Vec2f,Vec2f> >::iterator it2 = mFlowPoints.begin(); it2 != mFlowPoints.end(); ++it2 ){
					if( it2 != it1 ){
						Vec2f pos2 = it2->first;
						
						Vec2f dir = pos2 - pos1;
						float distSqrd = dir.lengthSquared();
						if( distSqrd < closestDist ){
							closestDist = distSqrd;
							secondClosestPos = closestPos;
							closestPos = pos2;
						}
						
						if( distSqrd < 100.0f ){
							mAnchors.push_back( pos1 );
							mAnchors.push_back( pos2 );
						}
					}
					
				}
				
				if( closestDist < 10000.0f ){
					mAnchors.push_back( pos1 );
					mAnchors.push_back( closestPos );
					
					if( secondClosestPos != Vec2f::zero() ){
						mAnchors.push_back( pos1 );
						mAnchors.push_back( secondClosestPos );
					}
				}
				
			}
		}
		
	}
	
	mGlobalDirLength = mGlobalDir.length() * 0.25f;
    
	for( vector<Particle>::iterator it = mSmallParticles.begin(); it != mSmallParticles.end(); )
	{
		if( it->isDead ){
			it = mSmallParticles.erase( it );
		} else {
			it->update();
			++it;
		}
	}
	
	for( vector<Particle>::iterator it = mLargeParticles.begin(); it != mLargeParticles.end(); )
	{
		if( it->isDead ){
			it = mLargeParticles.erase( it );
		} else {
			it->update();
			++it;
		}
	}
	
	for( vector<Particle>::iterator it = mSegments.begin(); it != mSegments.end(); )
	{
		if( it->isDead ){
			it = mSegments.erase( it );
		} else {
			it->update();
			++it;
		}
	}
}

void GFFTAdvApp::drawField( float x, float y )
{
    
    vector<float> coords;
    for(int i=0; i<mFlowPoints.size(); i++){
        coords.push_back( mFlowPoints[i].first.x + x );
        coords.push_back( mFlowPoints[i].first.y + y );
        coords.push_back( mFlowPoints[i].second.x + x );
        coords.push_back( mFlowPoints[i].second.y + y );
    }
	
	gl::color( ColorA( 1, 1, 1, 1 ) );
	
	//    glEnableClientState( GL_VERTEX_ARRAY );
	//    glVertexPointer( 2, GL_FLOAT, 0, &coords[0] );
	//    glDrawArrays( GL_LINES, 0, mFlowPoints.size() );
    //glDisableClientState(GL_VERTEX_ARRAY);
    
	//	gl::color( ColorA( 1, 1, 1, 0.1f ) );
	//    for( int i=0; i<coords.size(); i+=4 ){
	//        gl::drawStrokedCircle( Vec2f( coords[i], coords[i+1] ), 3.0f );
	//    }
}

void GFFTAdvApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::enableAlphaBlending();
	
	gl::pushMatrices();
	gl::scale( Vec2f( -2.0f, 2.0f ) );
	gl::translate( Vec2f( -getWindowWidth() * 0.5f, 0.0f ) );
	
	if( !mDrawOpenCvData )
		gl::color( Color( 1, 1, 1 ) );
	else
		gl::color( ColorA( 1, 1, 1, 0.4f ) );
	
	gl::draw( mCaptureTex );
	
	gl::color( Color( 1, 1, 1 ) );
	
	// DRAW PARTICLES
	glPointSize( 5.0f );
	glBegin( GL_POINTS );
	for( vector<Particle>::iterator it = mSmallParticles.begin(); it != mSmallParticles.end(); ++it ){
		it->draw();
	}
	glEnd();
	
	glPointSize( 10.0f );
	glBegin( GL_POINTS );
	for( vector<Particle>::iterator it = mLargeParticles.begin(); it != mLargeParticles.end(); ++it ){
		it->draw();
	}
	glEnd();
	
	// DRAW SEGMENTS
	glLineWidth( 2.0f );
	glBegin( GL_LINES );
	for( vector<Particle>::iterator it = mSegments.begin(); it != mSegments.end(); ++it ){
		it->drawLine();
	}
	glEnd();
	
	// DRAW LIGHT ANCHOR LINES
	gl::color( ColorA( 0.2f, 0.5f, 1.0f, math<float>::min( mGlobalDirLength * 0.1f, 0.1f ) ) );
	glLineWidth( 1.0f );
	glBegin( GL_LINES );
	for( int i=0; i<mAnchors.size(); ++i ){
		gl::vertex( mAnchors[i] );
	}
	glEnd();
	
	if( mDrawOpenCvData ){
		// DRAW CONNECTING LINES
		gl::color( ColorA( 0.2f, 0.5f, 1.0f, 0.6f ) );
		glBegin( GL_LINES );
		for(int i=0; i<mFlowPoints.size(); i++){
			Vec2f old( mFlowPoints[i].first );
			Vec2f pos( mFlowPoints[i].second );
			
			gl::vertex( old );
			gl::vertex( pos );
		}
		glEnd();
		
		// DRAW CURRENT POS
		gl::color( ColorA( 0.0f, 1.0f, 0.5f + mGlobalDirLength * 0.2f, 1.0f ) );
		for(int i=0; i<mFlowPoints.size(); i++){
			Vec2f pos( mFlowPoints[i].second );
			gl::drawSolidCircle( pos, 0.5f );
		}
		
		// DRAW PREVIOUS POS
		gl::color( ColorA( 1.0f, 0.0f, 0.5f, 1.0f ) );
		for(int i=0; i<mFlowPoints.size(); i++){
			Vec2f old( mFlowPoints[i].first );
			gl::drawSolidCircle( old, 0.5f );
		}
	}
	
	gl::popMatrices();
}




CINDER_APP_BASIC( GFFTAdvApp, RendererGl )
