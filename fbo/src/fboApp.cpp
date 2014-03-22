#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Camera.h"
#include "cinder/Arcball.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define FBO_WIDTH 800
#define FBO_HEIGHT 800

class fboApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void update();
	void draw();
    
    void renderSceneToFbo();
    CameraPersp cam;
    gl::Fbo mFbo;
    gl::GlslProg mShadowShader, mShader;
    
    Vec2i		mInitialMouseDown, mCurrentMouseDown;
};

void fboApp::setup()
{
    
    gl::Fbo::Format format;
    mFbo = gl::Fbo( FBO_WIDTH, FBO_HEIGHT, format );
    
    cam = CameraPersp( mFbo.getWidth(), mFbo.getHeight(), 60.0f );
    cam.setEyePoint( Vec3f( mFbo.getWidth(), mFbo.getHeight(), -300 ));
    cam.setPerspective( 60, mFbo.getAspectRatio(), 1, 10000 );
    
//	cam.lookAt( Vec3f( 2.8f, 1.8f, -2.8f ), Vec3f::zero() );
    
}

void fboApp::mouseDown( MouseEvent event )
{
	mInitialMouseDown = event.getPos();
}

void fboApp::mouseDrag( MouseEvent event )
{
    
    Vec3f e = cam.getEyePoint();
    e.z += mInitialMouseDown.y - event.getPos().y;
    cam.setEyePoint(e);
//    
//    if(mInitialMouseDown.y - event.y > 0)
//    {
//        Vec3f e = cam.getEyePoint();
//        e.z += mInitialMouseDown.y - event.y;
//        cam.setEyePoint(e);
//    }
//    else
//    {
//        Vec3f e = cam.getEyePoint();
//        e.z -= mInitialMouseDown.y - event.y;
//        cam.setEyePoint(e);
//    }
}

void fboApp::update()
{
	// render into our FBO
	renderSceneToFbo();
}


// Render the torus into the FBO
void fboApp::renderSceneToFbo()
{
    mFbo.bindFramebuffer();
    
    gl::clear( Colorf(0.25, 0.25, 0.25));
    
    gl::pushMatrices();
    
    gl::setMatrices(cam);
    gl::translate(400, 400, -1000);
    
    for( int i = 0; i < 10; i ++)
    {
        gl::color( float(i) / 10.f, 1.f - (float(i) / 10.f), float(i) / 10.f);
        gl::translate(0, 0, 100);
        gl::drawSphere(Vec3f( 0, 0, 0 ), 80);
        gl::drawSphere(Vec3f( 800, 800, 0 ), 80);
    }
    gl::popMatrices();
    
    mFbo.unbindFramebuffer();
}

void fboApp::draw()
{
    gl::clear();
    gl::color( 1.f, 1.f, 1.f );
    
    glEnable( GL_CULL_FACE );
    glCullFace(GL_FRONT);
    
    // simple
    //gl::draw( mFbo.getTexture(), Vec2f(0, 0) );
    
    // a little more advanced
    gl::pushMatrices();
    
    gl::translate( 300, 200, -100 );
    gl::rotate( Vec3f(0, getElapsedSeconds() * 4.f, 0) );

    mFbo.getTexture().enableAndBind();
    gl::drawCube(Vec3f( 0, 0, 0 ), Vec3f( 400, 400, 400 ));
    mFbo.getTexture().unbind();    
    
    gl::popMatrices();
}

CINDER_APP_NATIVE( fboApp, RendererGl )
