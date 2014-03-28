#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Rand.h"
#include "cinder/ObjLoader.h"
#include "cinder/MayaCamUI.h"
#include "cinder/params/Params.h"
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "FluidsModule.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class FluidsApp : public AppNative {
    
public:
    
	void prepareSettings( Settings *settings );
	void setup();
	void update();
	void draw();
    
    void keyDown( KeyEvent event );
    void mouseDown( ci::app::MouseEvent event );
    void mouseDrag( ci::app::MouseEvent event );
    void resize();
    
private:
    
    vector<FixtureRef>      mFixtures;
    
    gl::VboMeshRef          mFixtureMesh, mVenueMesh;   // 3d meshes
    
    ci::MayaCamUI           mMayaCam;                   // 3d camera
    
    FluidsModuleRef         mModule;                    // the effects
    
    params::InterfaceGlRef  mParams;
    
    float                   mFadeIn, mFadeOut;
    float                   mDissipation, mViscosity, mDensity, mForce, mRandomness;
    int                     mSpawnsN, mSourcesN;
};


void FluidsApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 1200, 800 );
}


void FluidsApp::setup()
{
    mFixtures		= Fixture::loadFixtures( getAssetPath("fixtures_001.csv") );						// load CSV fixtures file
    
    mFixtureMesh    = Fixture::loadObj( getAssetPath("sphere.obj") );                                   // load Fixture mesh
    mVenueMesh      = Fixture::loadObj( getAssetPath("venue.obj") );                                    // load Venue mesh
    
    mFadeIn         = 0.1f;
    mFadeOut        = 0.1f;
    
    mDissipation    = 0.95f;
    mViscosity      = 0.2f;
    mDensity        = 10.0f;
    mForce          = 0.15f;
    mRandomness     = 0.05f;
    mSpawnsN        = 10;
    mSourcesN       = 5;
    
    float rectSize  = 8.0f;
    Rectf rect      = Rectf( -rectSize, -rectSize, rectSize, rectSize );
    mModule         = FluidsModule::create( rect );                                 // create module
    
    mParams         = params::InterfaceGl::create( "Params", Vec2i( 200, 240 ) );   // Gui
    
    mParams->addParam( "Fade IN",       &mFadeIn,       "min=0.001 max=1.0 step=0.001" );
    mParams->addParam( "Fade OUT",      &mFadeOut,      "min=0.001 max=1.0 step=0.001" );
    mParams->addSeparator();
    
    mParams->addParam( "Dissipation",   &mDissipation,  "min=0.01 max=10.0 step=0.001" );
    mParams->addParam( "Viscosity",     &mViscosity,    "min=0.01 max=15.0 step=0.01" );
    mParams->addParam( "Density",       &mDensity,      "min=0.01 max=15.0 step=0.01" );
    mParams->addParam( "Force",         &mForce,        "min=0.01 max=15.0 step=0.01" );
    mParams->addParam( "Randomness",    &mRandomness,   "min=0.0 max=1.0 step=0.01" );
    mParams->addParam( "Spawns",        &mSpawnsN,      "min=0 max=100 step=1" );
    mParams->addParam( "Sources",       &mSourcesN,     "min=0 max=100 step=1" );
    
    ci::CameraPersp initialCam;                                                     // Initialise camera
    initialCam.setPerspective( 45.0f, ci::app::getWindowAspectRatio(), 0.1, 3000 );
    mMayaCam        = MayaCamUI( initialCam );
}


void FluidsApp::update()
{
    mModule->injectRandomFluid( mRandomness, mForce, mDensity, mSpawnsN );
    
    mModule->update( mFixtures, mDissipation, mViscosity );
    
    for( size_t k=0; k < mFixtures.size(); k++ )        // update fixtures brightness
        mFixtures[k]->update( mFadeIn, mFadeOut );
}


void FluidsApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::enableAlphaBlending();
    
    gl::setMatrices( mMayaCam.getCamera() );                   // set camera matrices
    
    Fixture::renderGrid();
    
    if ( mVenueMesh )                                           // render venue
    {
        gl::color( ColorA( 1.0f, 1.0f, 1.0f, 0.8f ) );
        gl::draw( mVenueMesh );
        
        gl::enableWireframe();
        gl::draw( mVenueMesh );
        gl::disableWireframe();
    }
    
    if ( mFixtureMesh )                                         // render fixtures
    {
        FixtureRef fix;
        
        for( size_t k=0; k < mFixtures.size(); k++ )
        {
            fix = mFixtures[k];
            
            ci::gl::pushMatrices();                             // store current matrices
            ci::gl::translate( fix->getPos() );                 // translate
            
            ci::gl::color( fix->getValue() * Color::white() );
            
            ci::gl::draw( mFixtureMesh );                       // draw mesh
            ci::gl::popMatrices();                              // restore matrices
        }
        
        ci::gl::color( ci::Color::white() );
    }
    
    if ( mModule )                                              // render effect preview
        mModule->render();
    
    mParams->draw();
}


void FluidsApp::keyDown( KeyEvent event )
{
    char c = event.getChar();
    
    if ( c == 'f' )
        setFullScreen( !isFullScreen() );
    
    if ( c == ' ' && mModule )
        mModule->createRandomSources( mSourcesN );
}


void FluidsApp::mouseDown( ci::app::MouseEvent event )
{
    if( event.isAltDown() )
        mMayaCam.mouseDown( event.getPos() );
}


void FluidsApp::mouseDrag( ci::app::MouseEvent event )
{
    if( event.isAltDown() )
        mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}


void FluidsApp::resize()
{
    ci::CameraPersp cam = mMayaCam.getCamera();
    cam.setPerspective( 45.0f, ci::app::getWindowAspectRatio(), 0.1, 3000 );
    mMayaCam.setCurrentCam( cam );
}


CINDER_APP_NATIVE( FluidsApp, RendererGl )
