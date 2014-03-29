#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/MayaCamUI.h"
#include "cinder/params/Params.h"
#include "cinder/gl/GlslProg.h"

#include "ciXtractReceiver.h"
#include "FrequenciesModule.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class FrequenciesApp : public AppNative {
    
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
    
    FrequenciesModuleRef    mModule;                    // the effects
    
    params::InterfaceGlRef  mParams;
    
    float                   mFadeIn, mFadeOut;
    float                   mSpread, mOffset, mBrightness;
    float                   mSoundGain, mSoundDamping, mSoundOffset;
    
    ciXtractReceiverRef     mXtract;                    // ciXtract receiver instance
    FeatureDataRef          mFeature;                   // the data received

	gl::GlslProgRef         mShader;
    
};


void FrequenciesApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 1200, 800 );
}


void FrequenciesApp::setup()
{	
	 mFixtures		= Fixture::loadFixtures( getAssetPath("fixtures_001.csv") );						// load CSV fixtures file
    
    vector<string> venues;
    venues.push_back("Sans_titre.obj");
    venues.push_back("t_dianae.obj");
    venues.push_back("amfiteatr.obj");
    
    for( int k=0; k < venues.size(); k++ )
        mVenueMeshes.push_back( Fixture::loadObj( getAssetPath( venues[k]) ) );			// load venues
    
    mFixtureMesh    = Fixture::loadObj( getAssetPath("sphere.obj") );					// load Fixture mesh
    mPianoMesh      = Fixture::loadObj( getAssetPath("piano.obj") );					// load Fixture mesh
    
    mVenueId        = 0;																// select the first one
    mVenueColor     = ColorA( 0.5f, 0.5f, 0.5f, 0.8f );
    mPianoColor     = ColorA( 0.1f, 0.1f, 0.1f, 0.8f );
    
    mFadeIn         = 0.5f;
    mFadeOut        = 0.1f;
    
    mSpread         = 1.0f;
    mOffset         = 0.0f;
    mBrightness     = 1.0f;
    mSoundGain      = 1.0f;
    mSoundDamping   = 0.95f;
    mSoundOffset    = 0.0f;
    
    mModule         = FrequenciesModule::create();                                  // create module
    
    mParams         = params::InterfaceGl::create( "Params", Vec2i( 200, 240 ) );   // Gui
    
    // venue objects
    mParams->addParam( "Venue", venues, (int*)&mVenueId );
    mParams->addParam( "Venue color",   &mVenueColor );
    mParams->addParam( "Piano color",   &mPianoColor );

    // fading
    mParams->addParam( "Fade IN",       &mFadeIn    , "min=0.001 max=1.0 step=0.001" );
    mParams->addParam( "Fade OUT",      &mFadeOut   , "min=0.001 max=1.0 step=0.001" );

    // module
    mParams->addSeparator();
    mParams->addParam( "Brightness",    &mBrightness, "min=0.1 max=15.0 step=0.1" );
    mParams->addParam( "Spread",        &mSpread    , "min=0.0 max=1.0 step=0.01" );
    mParams->addParam( "Offset",        &mOffset    , "min=0.0 max=1.0 step=0.01" );
    
    // sound
    mParams->addSeparator();
    mParams->addParam( "Sound Gain",    &mSoundGain     , "min=0.1 max=50.0 step=0.1" );
    mParams->addParam( "Sound Damping", &mSoundDamping  , "min=0.1 max=1.0 step=0.001" );
    mParams->addParam( "Sound Offset",  &mSoundOffset   , "min=0.1 max=1.0 step=0.001" );
    
    ci::CameraPersp initialCam;                                                     // Initialise camera
    initialCam.setPerspective( 45.0f, ci::app::getWindowAspectRatio(), 0.1, 3000 );
    mMayaCam        = MayaCamUI( initialCam );
    
    // init LibXtract
    mXtract = ciXtractReceiver::create();
    //	mFeature   = mXtract->getFeatureData( "XTRACT_BARK_COEFFICIENTS" );
	mFeature   = mXtract->getFeatureData( "XTRACT_SPECTRUM" );
    mFeature->setLog( true );
    
    // load the shader
    try {
		mShader = gl::GlslProg::create( loadAsset("shaders/simpleShader.vert"), loadAsset("shaders/simpleShader.frag") );
	}
	catch( gl::GlslProgCompileExc &exc ) {
		std::cout << "Shader compile error: " << std::endl;
		std::cout << exc.what();
	}
	catch( ... ) {
		std::cout << "Unable to load shader" << std::endl;
	}
}


void FrequenciesApp::update()
{
    if ( mFeature )
	{
		mFeature->setDamping( mSoundDamping );
		mFeature->setGain( mSoundGain );
		mFeature->setOffset( mSoundOffset );
	}
    
    mXtract->update();
    
    if ( mFeature )
        mModule->update( mFixtures, mFeature, mSpread, mOffset, mBrightness );
    
    for( size_t k=0; k < mFixtures.size(); k++ )            // update fixtures brightness
        mFixtures[k]->update( mFadeIn, mFadeOut );
}


void FrequenciesApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::enableAlphaBlending();
    
    gl::setMatrices( mMayaCam.getCamera() );                   // set camera matrices
    
    Fixture::renderGrid();
    
//    mShader->bind();	
    
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
    
//    mShader->unbind();
    
    gl::setMatricesWindow( getWindowSize() );
    gl::disableDepthRead();
    gl::disableDepthWrite();
    
    Rectf r = Rectf( 15, getWindowHeight() - 135, 215, getWindowHeight() - 35 );
    ciXtractReceiver::drawData( mFeature, r );
    
    mParams->draw();
}


void FrequenciesApp::keyDown( KeyEvent event )
{
    char c = event.getChar();
    
    if ( c == 'f' )
        setFullScreen( !isFullScreen() );
}


void FrequenciesApp::mouseDown( ci::app::MouseEvent event )
{
    if( event.isAltDown() )
        mMayaCam.mouseDown( event.getPos() );
}


void FrequenciesApp::mouseDrag( ci::app::MouseEvent event )
{
    if( event.isAltDown() )
        mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}


void FrequenciesApp::resize()
{
    ci::CameraPersp cam = mMayaCam.getCamera();
    cam.setPerspective( 45.0f, ci::app::getWindowAspectRatio(), 0.1, 3000 );
    mMayaCam.setCurrentCam( cam );
}


CINDER_APP_NATIVE( FrequenciesApp, RendererGl )
