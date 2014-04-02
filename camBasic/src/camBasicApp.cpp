#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/CinderMath.h"
#include "cinder/PolyLine.h"
#include "cinder/params/Params.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class camBasicApp : public AppNative {
  public:
	void setup();
	void update();
	void draw();
    
    void mouseDown( MouseEvent event );
    void mouseDrag( MouseEvent event );
    
	CameraPersp		mCam;
  MayaCamUI		mMayaCam;
    
  Vec3f           mSpin;
    params::InterfaceGl mControls;
    
    
    float           mFov, mAngle, mFront, mBack, mAspect;
    Vec3f           mLookAt, mPosition;

};

void camBasicApp::mouseDown(MouseEvent event)
{
    mMayaCam.mouseDown(event.getPos());
}

void camBasicApp::mouseDrag(MouseEvent event)
{
    mMayaCam.mouseDrag(event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown());
}

void camBasicApp::setup()
{
    mFov = 45;
    mAngle = 45.f;
    mFront = 0.1f;
    mBack = 100.f;
    mAspect = getWindowAspectRatio();
    
    mPosition = Vec3f(3,3,3);
    mLookAt = Vec3f(0,0,0);
    
    //mCam = CameraPersp( getWindowWidth(), getWindowHeight(), 45 );
	//mCam.setPerspective( 45.0f, getWindowAspectRatio(), 0.1f, 10000.0f );
    mCam.setPerspective( 45.0f, getWindowAspectRatio(), 0.1, 10000 );
	//mCam.lookAt( Vec3f( 3, 3, 3 ), Vec3f::zero() );
    
    mSpin = Vec3f(0,0,0);
    
    mControls = params::InterfaceGl(" Params ", Vec2i (200,150));
    mControls.addParam("fov", &mFov);
    mControls.addParam("angle", &mAngle);
    mControls.addParam("front", &mFront);
    mControls.addParam("back", &mBack);
    mControls.addParam("aspect", &mAspect);
    mControls.addParam("LookAt", &mLookAt);
    mControls.addParam("Position", &mPosition);

    // this is in setup
    mMayaCam.setCurrentCam(mCam);
    
}

void camBasicApp::update()
{
    
  //mCam.setPerspective( mAngle, mAspect, mFront, mBack );
	//mCam.lookAt( mPosition, mLookAt );
    
	// spin the scene by a few degrees around the y Axis
	mSpin.y += 1.f;
}

void camBasicApp::draw()
{
    
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	gl::enableAlphaBlending();
	
	gl::setMatrices( mMayaCam.getCamera() );
	gl::rotate( mSpin );
    
	// draw the globe
	gl::enableWireframe();
	gl::color( ColorA( 1, 1, 0, 0.25f ) );
	gl::drawSphere( Vec3f::zero(), 1, 20 );
  gl::drawSphere( Vec3f(0,0,10), 1, 20 );
	gl::disableWireframe();
    
    mControls.draw();
	
}

CINDER_APP_NATIVE( camBasicApp, RendererGl )
