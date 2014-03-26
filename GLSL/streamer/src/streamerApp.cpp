#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class streamerApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
    gl::GlslProg mShader;
    gl::Texture mTex;
};

void streamerApp::setup()
{
    mShader = gl::GlslProg( loadAsset("streamers.vs"), loadAsset("streamers.fs"));
    mTex = gl::Texture( loadImage( loadAsset("mona_lisa.jpg")));
}

void streamerApp::mouseDown( MouseEvent event )
{
}

void streamerApp::update()
{
}

void streamerApp::draw()
{
    
    gl::pushMatrices();
    
    gl::translate(Vec3f(getWindowWidth()/2, getWindowHeight()/2, 100));
    gl::rotate(Vec3f(0, getElapsedSeconds() * 10.0, 0));
    
	// clear out the window with black
	gl::clear( Color( 0.2, 0.2, 0.2 ) );
    
    glEnable(GL_CULL_FACE); // enables face culling
    glCullFace(GL_BACK); // tells OpenGL to cull back faces (the sane default setting)
    glFrontFace(GL_CW); // tells OpenGL which faces are considered 'front' (use GL_CW or GL_CCW)
    
    mShader.bind();
    mTex.enableAndBind();
    
    mShader.uniform("WireColor", Vec4f(1,1,1,1));
    mShader.uniform("EmptyColor", Vec4f(0.2,0.2,0.2,0.0));
    mShader.uniform("Balance", (float) fabs(sin(getElapsedSeconds())));
    mShader.uniform("time", (float) getElapsedSeconds()/20.f);
    mShader.uniform("tex", 0);
    
    gl::drawCube(Vec3f(0, 0, 0), Vec3f(200, 200, 200));

    mTex.unbind();
    mShader.unbind();
    
    gl::popMatrices();
}

CINDER_APP_NATIVE( streamerApp, RendererGl )
