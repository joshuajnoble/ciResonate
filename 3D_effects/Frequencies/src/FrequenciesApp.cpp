#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/ObjLoader.h"
#include "cinder/MayaCamUI.h"
#include "cinder/params/Params.h"
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

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
    
    void loadFixtures( string fileName, bool flipZ = true );
    
    gl::VboMeshRef loadObj( string fileName );
    
    void renderGrid();
    
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
    
    ciXtractReceiverRef     mXtract;			// ciXtract receiver instance
    FeatureDataRef          mFeature;				// the data received
    
};


void FrequenciesApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 1200, 800 );
}


void FrequenciesApp::setup()
{
    addAssetDirectory( "../../../../assets/" );
    
    loadFixtures( "fixtures_001.csv" );                                             // load CSV fixtures file
    
    vector<string> venues;
    venues.push_back("Sans_titre.obj");
    venues.push_back("t_dianae.obj");
    venues.push_back("amfiteatr.obj");
    
    for( int k=0; k < venues.size(); k++ )
        mVenueMeshes.push_back( loadObj( venues[k] ) );                             // load venues
    
    mFixtureMesh    = loadObj( "sphere.obj" );                                      // load Fixture mesh
    mPianoMesh      = loadObj( "piano.obj" );                                       // load Fixture mesh
    
    mVenueId        = 0;                                                            // select the first one
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
    
    // Init LibXtract
    mXtract = ciXtractReceiver::create();
    //	mFeature   = mXtract->getFeatureData( "XTRACT_BARK_COEFFICIENTS" );
	mFeature   = mXtract->getFeatureData( "XTRACT_SPECTRUM" );
    mFeature->setLog( true );
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
    
    renderGrid();
    
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
    
    gl::setMatricesWindow( getWindowSize() );
    gl::disableDepthRead();
    gl::disableDepthWrite();
    
    Rectf r = Rectf( 15, getWindowHeight() - 135, 215, getWindowHeight() - 35 );
    ciXtractReceiver::drawData( mFeature, r );
    
    mParams->draw();
}


void FrequenciesApp::loadFixtures( string fileName, bool flipZ )
{
    console() << "Load fixtures file: " << fileName << endl;
    
    mFixtures.clear();                                                                  // destroy fixtures
    
    fs::path filePath = getAssetPath( fileName );                                       // get asset path
    
    ifstream openFile( filePath.generic_string().c_str() );                             // open file stream
    
    ci::Vec3f   pos;
    std::string line;
    int         c = 0;
    
    if ( openFile.is_open() )                                                           // read file and parse comma separated values, one fixture per line
    {
        while ( openFile.good() )
        {
            getline(openFile,line);                                                     // get line
            
            std::vector<std::string> splitValues;                                       // split comma separated values
            boost::split(splitValues, line, boost::is_any_of(","));
            
            if ( splitValues.size() < 3 )                                               // we only import XYZ, check the line contains 3 values
            {
                console() << "Error parsing line #" << c << endl;
                console() << line << endl;
                
                c++;
                continue;
            }
            
            pos.x = boost::lexical_cast<float>(splitValues.at(0));                      // get X
            pos.y = boost::lexical_cast<float>(splitValues.at(1));                      // get Y
            pos.z = boost::lexical_cast<float>(splitValues.at(2));                      // get Z
            
            if ( flipZ ) pos.z *= -1;                                                   // sometimes we need to flip the Z coords
            
            
            mFixtures.push_back( Fixture::create( pos ) );    // create a new Fixture
            
            c++;
        }
        
        openFile.close();
    }
    else
        console() << "Failed loading file: " << filePath.generic_string() << endl;
    
    console() << "Loaded " << mFixtures.size() << " fixtures" << endl;
}


gl::VboMeshRef FrequenciesApp::loadObj( string fileName )
{
    gl::VboMeshRef  vboMesh;
    fs::path        filePath = getAssetPath( fileName );                                // get asset path
    
    
    ObjLoader loader( (DataSourceRef)loadFile( filePath ) );                            // load .obj file
    
    TriMesh	mesh;                                                                       // load TriMesh
    loader.load( &mesh );
    
    vboMesh = gl::VboMesh::create( mesh );                                              // create VBO mesh shared_ptr
    
    console() << "Loaded mesh: " << filePath.generic_string() << endl;
    
    return vboMesh;
}


void FrequenciesApp::renderGrid()
{
    int     steps           = 10;                               // sizes in meters
    float   size            = 1.0f;
    float   halfLineLength  = size * steps * 0.5f;              // half line length
    
    ci::gl::color( Color::gray( 0.4f ) );
    
    for( float i = -halfLineLength; i <= halfLineLength; i += size )
    {
        ci::gl::drawLine( ci::Vec3f( i, 0.0f, -halfLineLength ), ci::Vec3f( i, 0.0f, halfLineLength ) );
        ci::gl::drawLine( ci::Vec3f( -halfLineLength, 0.0f, i ), ci::Vec3f( halfLineLength, 0.0f, i ) );
    }
    
    ci::gl::drawCoordinateFrame();                              // draw axis
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
