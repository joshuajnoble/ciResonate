#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Capture.h"
#include "cinder/Arcball.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class camerasApp : public AppNative {
  public:
	void setup();
	void resize();
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void keyDown( KeyEvent event );
    void update();
	void draw();

	Arcball		mArcball;
	Vec2i		mInitialMouseDown, mCurrentMouseDown;
	bool		mDrawVerbose;
	bool		mUseConstraintAxis;
	Vec3f		mConstraintAxis;
    
    CaptureRef			mCapture;
	gl::TextureRef		mTexture;

};

void camerasApp::setup()
{

	glEnable( GL_CULL_FACE );
	glFrontFace( GL_CW ); // the default camera inverts to a clockwise front-facing direction

	mDrawVerbose = true;
	mUseConstraintAxis = false;
	mCurrentMouseDown = mInitialMouseDown = Vec2i( 200, 200 );
    
    mCapture = Capture::create( 320,240 );
    mCapture->start();

}

void camerasApp::resize()
{
	mArcball.setWindowSize( getWindowSize() );
	mArcball.setCenter( getWindowCenter() );
	mArcball.setRadius( 150 );
}


void camerasApp::mouseDown( MouseEvent event )
{
	mArcball.mouseDown( event.getPos() );
	mCurrentMouseDown = mInitialMouseDown = event.getPos();
}

void camerasApp::mouseDrag( MouseEvent event )
{
	mArcball.mouseDrag( event.getPos() );
	mCurrentMouseDown = event.getPos();
}

void camerasApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'v' )
		mDrawVerbose = ! mDrawVerbose;
	else if( event.getChar() == 'c' ) {
		mUseConstraintAxis = ! mUseConstraintAxis;
		if( mUseConstraintAxis ) {
			// make a random constraint axis
			mConstraintAxis = Rand::randVec3f();
			mArcball.setConstraintAxis( mConstraintAxis );
		}
		else
			mArcball.setNoConstraintAxis();
	}
}

void camerasApp::update()
{
    if( mCapture && mCapture->checkNewFrame() ) {
		mTexture = gl::Texture::create( mCapture->getSurface());
	}
}

void camerasApp::draw()
{
	gl::clear( Color( 0, 0.1f, 0.2f ) );
    
    // draw the cube
	gl::pushModelView();
    gl::translate( getWindowCenter() );
    gl::rotate( mArcball.getQuat() );
    if(mTexture) {
        mTexture->enableAndBind();
        gl::drawCube(Vec3f::zero(), Vec3f(320,320,320));
        mTexture->unbind();
    }
	gl::popModelView();
    


}

CINDER_APP_NATIVE( camerasApp, RendererGl )
