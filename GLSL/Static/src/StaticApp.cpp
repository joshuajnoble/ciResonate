#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class StaticApp : public AppNative {
  public:
	void setup();
	void mouseDrag( MouseEvent event );
    void mouseMove( MouseEvent event );
	void update();
	void draw();
    
    gl::GlslProg mShader;
    Vec2f mRes;
    Vec2f mRotation;
    float mDivisor;
    gl::Fbo	mFbo;
    
};

void StaticApp::setup()
{
    mRes.set(640, 640);//mRes.set(640, 640);
    try {
        mShader = gl::GlslProg(loadAsset("basic.vert"), loadAsset("static.frag"));
    } catch ( std::exception error ) {
        cout << error.what() << endl;
    }
    
    gl::Fbo::Format format;
    mFbo = gl::Fbo( 640, 640, format );
    
    // begin draw to FBO
	mFbo.bindFramebuffer();
    
    gl::color( Colorf( 1.0f, 0.5f, 0.25f ) );
    gl::drawSolidRect(Rectf(0, 0, 640, 640));
    
    // end draw to FBO
	mFbo.unbindFramebuffer();
    mDivisor = 64.f;
    mRotation.set(0.1,1.5);
}

void StaticApp::mouseMove( MouseEvent event )
{
    float x = (event.getX() / 320.f);
    float y = (event.getY() / 240.f);
    
    mRotation.set(x,y);
}

void StaticApp::mouseDrag( MouseEvent event )
{
    mDivisor = event.getY();
}

void StaticApp::update()
{
}

void StaticApp::draw()
{
    
    // clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    mShader.bind();
    
    mShader.uniform("time", (float) getElapsedSeconds() * 2.f);
    mShader.uniform("res", mRes);
    mShader.uniform("divisor", mDivisor);
    mShader.uniform("rotation", mRotation);
    
    // use the scene we rendered into the FBO as a texture
    glEnable(GL_TEXTURE_2D);
	mFbo.bindTexture();
    
    gl::draw( mFbo.getTexture(0), Rectf( 0, 0, 640, 640 ) );
    
    mShader.unbind();
    
}

CINDER_APP_NATIVE( StaticApp, RendererGl )
