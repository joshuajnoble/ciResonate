#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class HairShadingApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();

	gl::GlslProgRef mHairShader;
};

void HairShadingApp::setup()
{
	mHairShader = gl::GlslProg::create( loadAsset("fur.vert"), loadAsset("fur.geom"), loadAsset("fur.frag"));
}

void HairShadingApp::mouseDown( MouseEvent event )
{
}

void HairShadingApp::update()
{
}

void HairShadingApp::draw()
{
	mHairShader->bind();
	mHairShader->uniform("u_projectionMatrix", gl::getProjection());
	mHairShader->uniform("u_modelViewMatrix", gl::getModelView());
	//mHairShader->uniform("u_normalMatrix");

}

CINDER_APP_NATIVE( HairShadingApp, RendererGl )
