#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Camera.h"
#include "cinder/ObjLoader.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define FBO_WIDTH 800
#define FBO_HEIGHT 600

class vboApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
    CameraPersp cam;
    gl::VboMesh mVbo;
    TriMesh mTriMesh;
    gl::GlslProg mShadowShader, mShader;
};

void vboApp::setup()
{
    
    ObjLoader loader(loadAsset("teapot.obj"));
    loader.load(&mTriMesh);
    
    gl::VboMesh::Layout layout;
	layout.setStaticIndices();
	layout.setStaticPositions();
    layout.setDynamicColorsRGB();
	//layout.setStaticTexCoords2d();
    
    mVbo = gl::VboMesh( mTriMesh, layout );
    setWindowSize(1200, 800);
}

void vboApp::mouseDown( MouseEvent event )
{
}

void vboApp::update()
{
 
    // update the colors every frame?
    Colorf hsb = Colorf(CM_HSV, 0.f, 1.f, 1.f);
    float hue = fabs( sin(getElapsedSeconds()/10.f)/2.f );
    // dynamically generate our new colors
	gl::VboMesh::VertexIter iter = mVbo.mapVertexBuffer();
	while( iter.isDone() )
    {
        hue += 0.0001;
        hsb.set( CM_HSV, Vec3f( min<float>( hue, 1.f), 1.f, 1.f));
        iter.setColorRGB(hsb);
        ++iter;
    }
}

void vboApp::draw()
{
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    gl::clear( Colorf(0.3, 0.3, 0.3));
    gl::pushMatrices();
    for( int i = 0; i < 10; i++ )
    {
        for( int j = 0; j < 10; j++ )
        {
            gl::translate( 120, 0 );
            // make the teapot face forward
            gl::rotate(Vec3f(180,0,0));
            gl::draw(mVbo);
            gl::rotate(Vec3f(-180,0,0));
        }
        gl::translate( -1200, 100 );
    }
    gl::popMatrices();
}

CINDER_APP_NATIVE( vboApp, RendererGl )
