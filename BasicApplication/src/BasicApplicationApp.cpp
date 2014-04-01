#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/TriMesh.h"
#include "cinder/gl/Texture.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicApplicationApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();

	TriMesh plane;
    gl::Texture mTexture;
};

void BasicApplicationApp::setup()
{

	plane.appendVertex(Vec3f(0, 0, 0)); // [ (0,0,0) ]
    plane.appendColorRgba(Colorf(1.f,0,0));
    plane.appendTexCoord(Vec2f(0,0));
    
    plane.appendVertex(Vec3f(600, 0, 0)); // [ (0,0,0), (600,0,0) ]
    plane.appendColorRgba(Colorf(1.f,1.f,0));
    plane.appendTexCoord(Vec2f(1.f,0));
    
    plane.appendVertex(Vec3f(600, 600, 0)); // [ (0,0,0), (600,0,0), (600,600,0)]
    plane.appendColorRgba(Colorf(0,1.f,0));
    plane.appendTexCoord(Vec2f(1.f,1.f));
    
    plane.appendVertex(Vec3f(0, 600, 0)); // [ (0,0,0), (600,0,0), (600,600,0), (0,600,0)]
    plane.appendColorRgba(Colorf(0,0,1.f));
    plane.appendTexCoord(Vec2f(0.f,1.f));
    
    uint indices[6] = {0,1,2,2,3,0};
    plane.appendIndices(&indices[0], 6);
    
    mTexture = gl::Texture( loadImage()))

}

void BasicApplicationApp::mouseDown( MouseEvent event )
{
}

void BasicApplicationApp::update()
{
}

void BasicApplicationApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::draw(plane);
    
    
}

CINDER_APP_NATIVE( BasicApplicationApp, RendererGl )
