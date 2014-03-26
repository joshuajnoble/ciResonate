#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/TriMesh.h"
#include "cinder/ImageIo.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class glitchApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );
    void mouseMove( MouseEvent event );
	void update();
	void draw();
    
    gl::GlslProg mShader;
    gl::Texture mTex;
    Vec2f mouse;
    Vec2f resolution;
};

void glitchApp::setup()
{
    try
    {
        mShader = gl::GlslProg(loadAsset("scratch.vert"), loadAsset("scratch.frag"));
    }
    catch ( gl::GlslProgCompileExc err )
    {
        cout << err.what() << endl;
    }
    
    resolution.set(800,600);
}

void glitchApp::mouseMove(MouseEvent event)
{
    mouse = event.getPos();
}

void glitchApp::mouseDown( MouseEvent event )
{
}

void glitchApp::update()
{
}

void glitchApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    gl::pushMatrices();
    
	if ( !mShader )
        return;
    
    //mTex.enableAndBind();
    
    mShader.bind();
    mShader.uniform("resolution", resolution);
    mShader.uniform("mouse", mouse);
    mShader.uniform("time", (float) getElapsedSeconds());
//    mShader.uniform("timeValX", (float) getElapsedSeconds());
//    mShader.uniform("timeValY", (float) getElapsedSeconds());
    
    gl::drawSolidRect( getWindowBounds() );
    //mTex.unbind();
    mShader.unbind();
    gl::popMatrices();
}

CINDER_APP_NATIVE( glitchApp, RendererGl )
