#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Exception.h"
#include "cinder/gl/Texture.h"
#include "cinder/ObjLoader.h"
#include "cinder/Matrix.h"
#include "cinder/Arcball.h"
#include "cinder/MayaCamUI.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class HairShadingApp : public AppNative {
  public:
	void setup();
    void resize();
    void mouseDrag( MouseEvent event );
	void mouseDown( MouseEvent event );
    void keyDown(cinder::app::KeyEvent event);
	void update();
	void draw();

	gl::GlslProg mHairShader;
    gl::Texture mTex;
    TriMesh	mMesh;
    Arcball			mArcball;
	MayaCamUI		mMayaCam;
    
    float mHairLength;
};

void HairShadingApp::setup()
{
    
    mHairLength = 4.f;
    
    ObjLoader loader( (DataSourceRef) loadAsset( "largebunny.obj" ) );
	loader.load( &mMesh );
    
    try {
        mHairShader = gl::GlslProg( loadAsset("fur2.vert"), loadAsset("fur2.frag"), loadAsset("fur2.geom"), GL_TRIANGLES, GL_TRIANGLE_STRIP, 3);
        
    } catch (gl::GlslProgCompileExc & e) {
        cout << e.what() << endl;
    }
    mTex = gl::Texture(loadImage(loadAsset("hair.jpg")));
    
    CameraPersp initialCam;
	initialCam.setPerspective( 45.0f, getWindowAspectRatio(), 0.1, 10000 );
    Vec3f eye(0.75,10,-44);
    initialCam.setEyePoint( eye );
    Vec3f look(0,0.7,1);
    initialCam.setViewDirection( look );
    mMayaCam.setCurrentCam( initialCam );
}

void HairShadingApp::resize()
{
    mArcball.setWindowSize( getWindowSize() );
	mArcball.setCenter( Vec2f( getWindowWidth() / 2.0f, getWindowHeight() / 2.0f ) );
	mArcball.setRadius( 150 );
}

void HairShadingApp::keyDown(cinder::app::KeyEvent event)
{
    if(event.getChar() == '=') {
        mHairLength+=1.0;
    }
    if(event.getChar() == '-') {
        mHairLength-=1.0;
    }
}

void HairShadingApp::mouseDown( MouseEvent event )
{
    if( event.isAltDown() ) {
        mMayaCam.mouseDown( event.getPos() );
	} else {
        mArcball.mouseDown( event.getPos() );
    }
}

void HairShadingApp::mouseDrag(cinder::app::MouseEvent event)
{
    if( event.isAltDown() ) {
        mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
    } else {
        mArcball.mouseDrag( event.getPos() );
    }
}

void HairShadingApp::update()
{
}

void HairShadingApp::draw()
{
    
    gl::enableDepthWrite();
	gl::enableDepthRead();
    gl::enableAlphaBlending();
	
	gl::clear( Color( 1.0f, 1.0f, 1.0f ) );
    gl::pushMatrices();
    
    //gl::scale(20.f,20.f,20.f);
    
    gl::setMatrices( mMayaCam.getCamera() );
    
    mTex.enableAndBind();
	mHairShader.bind();
    gl::rotate( mArcball.getQuat() );
    
	mHairShader.uniform("u_projectionMatrix", gl::getProjection());
	mHairShader.uniform("u_modelViewMatrix", gl::getModelView());
    mHairShader.uniform("hairLength", mHairLength);
    gl::draw(mMesh);
    
    mTex.unbind();
    mHairShader.unbind();
    
    gl::popMatrices();

}

CINDER_APP_NATIVE( HairShadingApp, RendererGl )
