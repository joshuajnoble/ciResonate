
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/TriMesh.h"
#include "cinder/ImageIo.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class SimpleShadingApp : public AppNative {
public:
	void setup();
    void mouseMove( MouseEvent event );
	void update();
	void draw();
    
    gl::GlslProg mShader;
    gl::Texture mTex;
    float mMouseX;
    
};



void SimpleShadingApp::setup()
{
    try
    {
        mShader = gl::GlslProg(loadAsset("shader.vert"), loadAsset("shader.frag"));
    }
    catch ( std::exception error )
    {
        console() << error.what() << endl;
    }
    
    gl::Texture::Format format;
    format.setTarget(GL_TEXTURE_RECTANGLE_ARB);
    mTex = gl::Texture(loadImage(loadAsset("test.jpg")), format);
    
}

void SimpleShadingApp::mouseMove( MouseEvent event )
{
    mMouseX = event.getX();
}

void SimpleShadingApp::update()
{
}


void SimpleShadingApp::draw()
{

    // clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    gl::pushMatrices();

	if ( !mShader )
		return;

    mTex.enableAndBind();
    
    mShader.bind();
    mShader.uniform("mouseX", mMouseX);
    mShader.uniform("tex0", 0);
    
    gl::drawSolidRect( getWindowBounds() );
    mTex.unbind();
    mShader.unbind();
    gl::popMatrices();
}

CINDER_APP_NATIVE( SimpleShadingApp, RendererGl )
