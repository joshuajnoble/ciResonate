#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/ObjLoader.h"
#include "cinder/MayaCamUI.h"
#include "cinder/params/Params.h"
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "SpawnModule.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class SpawnApp : public AppNative {
    
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
    
    gl::VboMeshRef          mFixtureMesh;               // fixture mesh
    gl::VboMeshRef          mPianoMesh;                 // fixture mesh
    vector<gl::VboMeshRef>  mVenueMeshes;
    gl::VboMeshRef          mSelectedVenueMesh;
    size_t                  mVenueId;
    
    ColorA                  mVenueColor;
    ColorA                  mPianoColor;
    
    ci::MayaCamUI           mMayaCam;                   // 3d camera
    
    SpawnModuleRef          mModule;                    // the effects
    
    params::InterfaceGlRef  mParams;
    
    float                   mFadeIn, mFadeOut;
    
    int                     mPointsN, mDeg;
    float                   mRadius, mSpeed;
    
};


void SpawnApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 1200, 800 );
}


void SpawnApp::setup()
{
	mFixtures =	Fixture::loadFixtures( getAssetPath("fixtures_001.csv") );              // load CSV fixtures file
    
    vector<string> venues;
    venues.push_back("Sans_titre.obj");
    venues.push_back("t_dianae.obj");
    venues.push_back("amfiteatr.obj");
    
    for( int k=0; k < venues.size(); k++ )
        mVenueMeshes.push_back( Fixture::loadObj( getAssetPath(venues[k]) ) );          // load venues
    
    mFixtureMesh    = Fixture::loadObj( getAssetPath("sphere.obj") );                   // load Fixture mesh
    mPianoMesh      = Fixture::loadObj( getAssetPath("piano.obj") );                    // load Fixture mesh
    
    mVenueId        = 0;																// select the first one
    mVenueColor     = ColorA( 0.0f, 0.5f, 0.5f, 0.8f );
    mPianoColor     = ColorA( 0.0f, 0.0f, 0.0f, 0.8f );
    
    mFadeIn         = 0.5f;
    mFadeOut        = 0.1f;
    
    
    mPointsN        = 5;
    mRadius         = 2.0f;
    mSpeed          = 0.1f;
    mDeg            = 3;
    
    mModule         = SpawnModule::create();                                        // create module
    
    mParams         = params::InterfaceGl::create( "Params", Vec2i( 200, 240 ) );   // Gui
    
    mParams->addParam( "Venue", venues, (int*)&mVenueId );
    mParams->addParam( "Venue color", &mVenueColor );
    mParams->addParam( "Piano color", &mPianoColor );
    
    mParams->addParam( "Fade IN",   &mFadeIn    , "min=0.001 max=1.0 step=0.001" );
    mParams->addParam( "Fade OUT",  &mFadeOut   , "min=0.001 max=1.0 step=0.001" );
    mParams->addSeparator();
    
    mParams->addParam( "Speed",     &mSpeed     , "min=0.001 max=10.0 step=0.001" );
    mParams->addParam( "Radius",    &mRadius    , "min=0.1 max=15.0 step=0.1" );
    mParams->addParam( "Points N",  &mPointsN   , "min=2 max=100 step=1" );
    mParams->addParam( "Deg",       &mDeg       , "min=1 max=10 step=1" );
    
    ci::CameraPersp initialCam;                                                     // Initialise camera
    initialCam.setPerspective( 45.0f, ci::app::getWindowAspectRatio(), 0.1, 3000 );
    mMayaCam        = MayaCamUI( initialCam );
}


void SpawnApp::update()
{
    mModule->update( mFixtures, mSpeed );
    
    for( size_t k=0; k < mFixtures.size(); k++ )        // update fixtures brightness
        mFixtures[k]->update( mFadeIn, mFadeOut );
}


void SpawnApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::enableAlphaBlending();
    
    gl::setMatrices( mMayaCam.getCamera() );                   // set camera matrices
    
    Fixture::renderGrid();
    
    if ( mVenueMeshes[mVenueId] )                              // render venue
    {
        gl::color( mVenueColor );
        gl::draw( mVenueMeshes[mVenueId] );
        
        gl::enableWireframe();
        gl::draw( mVenueMeshes[mVenueId] );
        gl::disableWireframe();
    }
    
    if ( mPianoMesh )                                           // render piano
    {
        gl::color( mPianoColor );
        gl::draw( mPianoMesh );
        
        gl::enableWireframe();
        gl::draw( mPianoMesh );
        gl::disableWireframe();
    }
    
    gl::color( Color::white() );
    
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


void SpawnApp::keyDown( KeyEvent event )
{
    char c = event.getChar();
    
    if ( c == 'f' )
        setFullScreen( !isFullScreen() );
    
    else if ( c == ' ' && mModule )
        mModule->createPath( mPointsN, mDeg, mRadius );
}


void SpawnApp::mouseDown( ci::app::MouseEvent event )
{
    if( event.isAltDown() )
        mMayaCam.mouseDown( event.getPos() );
}


void SpawnApp::mouseDrag( ci::app::MouseEvent event )
{
    if( event.isAltDown() )
        mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}


void SpawnApp::resize()
{
    ci::CameraPersp cam = mMayaCam.getCamera();
    cam.setPerspective( 45.0f, ci::app::getWindowAspectRatio(), 0.1, 3000 );
    mMayaCam.setCurrentCam( cam );
}


CINDER_APP_NATIVE( SpawnApp, RendererGl )


