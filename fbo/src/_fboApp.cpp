#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Camera.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define FBO_WIDTH 800
#define FBO_HEIGHT 600

class fboApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
    void renderSceneToFbo();
    CameraPersp cam, shadowCam;
    gl::Fbo mShadowFbo, mFbo;
    Matrix44f mTorusRotation, mShadowWVP;
    gl::GlslProg mShadowShader, mShader;
};

void fboApp::setup()
{
    
    gl::Fbo::Format format;
    mFbo = gl::Fbo( FBO_WIDTH, FBO_HEIGHT, format );
    
    gl::Fbo::Format shadowFormat;
    shadowFormat.enableDepthBuffer();
    mShadowFbo = gl::Fbo( FBO_WIDTH, FBO_HEIGHT, shadowFormat );

    mShadowShader = gl::GlslProg( loadAsset("shadow_map.vs"));
    mShader = gl::GlslProg( loadAsset("shadow_map.vs"), loadAsset("shadow_map.fs"));
    
	gl::enableDepthRead();
	gl::enableDepthWrite();
    
	mTorusRotation.setToIdentity();
    
    // setup our camera to represent the location of the light first
    shadowCam = CameraPersp( mFbo.getWidth(), mFbo.getHeight(), 60.0f );
    shadowCam.setEyePoint( Vec3f( 0, 1.8, 2.8));
	shadowCam.setPerspective( 60, mFbo.getAspectRatio(), 1, 1000 );
	shadowCam.lookAt( Vec3f( 1.8f, 1.8f, -2.8f ), Vec3f::zero() );

    cam = CameraPersp( mFbo.getWidth(), mFbo.getHeight(), 60.0f );
    cam.setEyePoint( Vec3f( 1.8, 1.8, -2.8));
    cam.setPerspective( 60, mFbo.getAspectRatio(), 1, 1000 );
	cam.lookAt( Vec3f( 2.8f, 1.8f, -2.8f ), Vec3f::zero() );
    
}

void fboApp::mouseDown( MouseEvent event )
{
}

void fboApp::update()
{
	// render into our FBO
	renderSceneToFbo();
    
    
    Vec3f v = shadowCam.getEyePoint();
    v.x += 0.5;
    shadowCam.setEyePoint( v );
}


// Render the torus into the FBO
void fboApp::renderSceneToFbo()
{
	
	// bind the framebuffer - now everything we draw will go there
	mShadowFbo.bindFramebuffer();
    
    mShadowShader.bind();
    
    //glClear(GL_DEPTH_BUFFER_BIT);
    gl::clear();
    
	// setup the viewport to match the dimensions of the FBO
	gl::setViewport( mFbo.getBounds() );
    
    gl::setMatrices( shadowCam );
	
	// clear out the FBO with blue
	gl::clear( Color( 0.25, 0.5f, 1.0f ) );
    
	// render an orange torus, with no textures
	//glDisable( GL_TEXTURE_2D );
	gl::color( Color( 1.0f, 0.5f, 0.25f ) );
	gl::drawTorus( 1.4f, 0.3f, 32, 64 );
    //	gl::drawColorCube( Vec3f::zero(), Vec3f( 2.2f, 2.2f, 2.2f ) );
    
    mShadowShader.unbind();
    
    mShadowFbo.unbindFramebuffer();
}

void fboApp::draw()
{
	// clear the window to gray
	gl::clear( Color( 0.35f, 0.35f, 0.35f ) );
    
	// setup our camera to render the cube
	cam.lookAt( Vec3f( 2.6f, 1.6f, -2.6f ), Vec3f::zero() );
	gl::setMatrices( cam );
    
	// set the viewport to match our window
	gl::setViewport( getWindowBounds() );
    
	mShader.bind();
    
    mShadowFbo.bindDepthTexture();
    mShader.uniform("gShadowMap", 0);
    
    gl::color( Color( 1.0f, 0.5f, 0.25f ) );
	gl::drawTorus( 1.4f, 0.3f, 32, 64 );
    
    mShader.unbind();
}

CINDER_APP_NATIVE( fboApp, RendererGl )
