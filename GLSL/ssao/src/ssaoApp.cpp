
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/params/Params.h"
#include "cinder/Rand.h"
#include "cinder/Text.h"
#include "cinder/Font.h"
#include "cinder/ObjLoader.h"

#include "boost/function.hpp"
#include "boost/bind.hpp"
#include "boost/lambda/lambda.hpp"

#include "DeferredRenderer.h"
#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define NUM_LIGHTS 100;        //number of lights


class ssaoApp : public AppNative
{
public:
    void prepareSettings( Settings *settings );
    void setup();
    void update();
    void draw();
    void keyDown( app::KeyEvent event );
    void mouseDown( MouseEvent event );
    void mouseDrag( MouseEvent event );
    
    void drawShadowCasters(gl::GlslProg* deferShader) const;
    void drawNonShadowCasters(gl::GlslProg* deferShader) const;
    void drawOverlay() const;
    
protected:
    int RENDER_MODE;
    
    //debug
    // cinder::params::InterfaceGl mParams;
    // bool                mShowParams;
    // float               mCurrFramerate;
    
    //camera
    MayaCamUI           mMayaCam;   //need a camera that will allow mCam to mirror it as MayaCamUI doesn't current allow a non const reference to its CameraPersp
    CameraPersp         mCam;
    DeferredRenderer    mDeferredRenderer;
    
    int mCurrLightIndex;
    gl::VboMeshRef         shadowPlane;
    TriMesh plane;
    
    gl::VboMesh       bunny;
    float mBunnyZ, mBunnyX;
    
};

void ssaoApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 1024, 768 );
    
    mBunnyZ = 30;
    mBunnyX = 30;
    
}



void ssaoApp::setup()
{
    
    gl::disableVerticalSync(); //so I can get a true representation of FPS (if higher than 60 anyhow :/)
    
    RENDER_MODE = DeferredRenderer::SHOW_FINAL_VIEW;
    
//    mParams = params::InterfaceGl( "3D_Scene_Base", Vec2i( 225, 125 ) );
//    mParams.addParam( "Framerate", &mCurrFramerate, "", true );
//    mParams.addParam( "Selected Light Index", &mCurrLightIndex);
//    mParams.addParam( "Show/Hide Params", &mShowParams, "key=x");
//    mParams.addSeparator();
//    
//    mCurrFramerate = 0.0f;
//    mShowParams = true;
    
    //set up camera
    mCam.setPerspective( 45.0f, getWindowAspectRatio(), 0.1f, 10000.0f );
    Vec3f camPos( -14.0f, 7.0f, -14.0f );
    mCam.lookAt(camPos * 1.5f, Vec3f::zero(), Vec3f(0.0f, 1.0f, 0.0f) );
    mCam.setCenterOfInterestPoint(Vec3f::zero());
    mMayaCam.setCurrentCam(mCam);
    
    //create functions pointers to send to deferred renderer
    boost::function<void(gl::GlslProg*)> fRenderShadowCastersFunc = boost::bind( &ssaoApp::drawShadowCasters, this, boost::lambda::_1 );
    boost::function<void(gl::GlslProg*)> fRenderNotShadowCastersFunc = boost::bind( &ssaoApp::drawNonShadowCasters, this,  boost::lambda::_1 );
    boost::function<void(void)> fRenderOverlayFunc = boost::bind( &ssaoApp::drawOverlay, this );

    mDeferredRenderer.setup( fRenderShadowCastersFunc, fRenderNotShadowCastersFunc, NULL, NULL, &mCam, Vec2i(1024, 768), 1024, true, true ); //no overlay or "particles"
    
    //have these cast point light shadows
    mDeferredRenderer.addCubeLight(    Vec3f(-2.0f, 4.0f, 6.0f),      Color(0.10f, 0.69f, 0.93f) * LIGHT_BRIGHTNESS_DEFAULT, true);      //blue
    mDeferredRenderer.addCubeLight(    Vec3f(4.0f, 6.0f, -4.0f),      Color(0.94f, 0.15f, 0.23f) * LIGHT_BRIGHTNESS_DEFAULT, true);      //red
    
    //add a bunch of lights
    for(int i = 0; i < 10; i++) {
        for(int j = 0; j < 10; j++) {
        
            int randColIndex = Rand::randInt(5);
            Color randCol;
            switch( randColIndex ) {
                case 0:
                    randCol = Color(0.99f, 0.67f, 0.23f); //orange
                    break;
                case 1:
                    randCol = Color(0.97f, 0.24f, 0.85f); //pink
                    break;
                case 2:
                    randCol = Color(0.00f, 0.93f, 0.30f); //green
                    break;
                case 3:
                    randCol = Color(0.98f, 0.96f, 0.32f); //yellow
                    break;
                case 4:
                    randCol = Color(0.10f, 0.69f, 0.93f); //blue
                    break;
                case 5:
                    randCol = Color(0.94f, 0.15f, 0.23f); //red
                    break;
            };
            
            mDeferredRenderer.addCubeLight( Vec3f( i * 20, 30, j * 20), randCol * LIGHT_BRIGHTNESS_DEFAULT, false, true);
        }
    }
    
    mCurrLightIndex = 0;
    
    float size = 3000;
    
    plane.appendVertex(Vec3f(size, -1,-size));
    plane.appendColorRgba(ColorA(255,255,255,255));
    plane.appendNormal(Vec3f(.0f, 1.0f, 0.0f));
    plane.appendVertex(Vec3f(-size, -1,-size));
    plane.appendColorRgba(ColorA(255,255,255,255));
    plane.appendNormal(Vec3f(.0f, 1.0f, 0.0f));
    plane.appendVertex(Vec3f(-size, -1, size));
    plane.appendColorRgba(ColorA(255,255,255,255));
    plane.appendNormal(Vec3f(.0f, 1.0f, 0.0f));
    plane.appendVertex(Vec3f(size, -1, size));
    plane.appendColorRgba(ColorA(255,255,255,255));
    plane.appendNormal(Vec3f(.0f, 1.0f, 0.0f));
    
    uint indices[6] = {0,1,2,2,3,0};
    plane.appendIndices(&indices[0], 6);
    
    gl::VboMesh::Layout layout;
    shadowPlane = gl::VboMesh::create(plane);
    
    TriMesh bunnyMesh;
    ObjLoader loader( loadResource(RES_BUNNY) );
	loader.load( &bunnyMesh );
    
    bunny = gl::VboMesh(bunnyMesh);
    
}

void ssaoApp::update()
{
    mCam = mMayaCam.getCamera();
    mDeferredRenderer.mCam = &mCam;
    //mCurrFramerate = getAverageFps();
    
    float base = getElapsedSeconds();
    
    vector<Light_PS*>::iterator it = mDeferredRenderer.getCubeLightsRef()->begin();
    for(it; it != mDeferredRenderer.getCubeLightsRef()->end(); ++it) {
        
        Vec3f v = (*it)->getPos();
        v.y = (40 + (sin(base) * 30.f));
        (*it)->setPos(v);
        
        base += 1.f;
        
    }
    
}

void ssaoApp::draw()
{
    mDeferredRenderer.renderFullScreenQuad(RENDER_MODE);
}

void ssaoApp::keyDown( KeyEvent event )
{
    float lightMovInc = 0.1f;
    
    switch ( event.getCode() )
    {
            //switch between render views
        case KeyEvent::KEY_0:
        {RENDER_MODE = DeferredRenderer::SHOW_FINAL_VIEW;}
            break;
        case KeyEvent::KEY_1:
        {RENDER_MODE = DeferredRenderer::SHOW_DIFFUSE_VIEW;}
            break;
        case KeyEvent::KEY_2:
        {RENDER_MODE = DeferredRenderer::SHOW_NORMALMAP_VIEW;}
            break;
        case KeyEvent::KEY_3:
        {RENDER_MODE = DeferredRenderer::SHOW_DEPTH_VIEW;}
            break;
        case KeyEvent::KEY_4:
        {RENDER_MODE = DeferredRenderer::SHOW_POSITION_VIEW;}
            break;
        case KeyEvent::KEY_5:
        {RENDER_MODE = DeferredRenderer::SHOW_ATTRIBUTE_VIEW;}
            break;
        case KeyEvent::KEY_6:
        {RENDER_MODE = DeferredRenderer::SHOW_SSAO_VIEW;}
            break;
        case KeyEvent::KEY_7:
        {RENDER_MODE = DeferredRenderer::SHOW_SSAO_BLURRED_VIEW;}
            break;
        case KeyEvent::KEY_8:
        {RENDER_MODE = DeferredRenderer::SHOW_LIGHT_VIEW;}
            break;
        case KeyEvent::KEY_9:
        {RENDER_MODE = DeferredRenderer::SHOW_SHADOWS_VIEW;}
            break;   
        case KeyEvent::KEY_a :
            mBunnyX-=0.5;
            break;
        case KeyEvent::KEY_w :
            mBunnyZ+=0.5;
            break;
        case KeyEvent::KEY_s :
            mBunnyZ-=0.5;
            break;
        case KeyEvent::KEY_d :
            mBunnyX+=0.5;
            break;
        
        default:
            break;
    }
}

void ssaoApp::mouseDown( MouseEvent event )
{
    if( event.isAltDown() ) {
        mMayaCam.mouseDown( event.getPos() );
    }
}

void ssaoApp::mouseDrag( MouseEvent event )
{
    if( event.isAltDown() ) {
        mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
    }
}

// all the things that will cast a shadow
void ssaoApp::drawShadowCasters(gl::GlslProg* deferShader) const
{
    gl::pushMatrices();
    gl::translate( Vec3f( mBunnyX, 0, mBunnyZ ));
    gl::draw(bunny);
    gl::popMatrices();
}

// things that have shadows cast on them
void ssaoApp::drawNonShadowCasters(gl::GlslProg* deferShader) const
{
    gl::draw(shadowPlane);
}

// anything you want to not be in the scene
void ssaoApp::drawOverlay() const
{
}

CINDER_APP_NATIVE( ssaoApp, RendererGl )
