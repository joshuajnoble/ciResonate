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

#include "Fixture.h"
#include "BlobModule.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class BlobApp : public AppNative {

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

    gl::VboMeshRef          mFixtureMesh, mVenueMesh;   // 3d meshes
    
    ci::MayaCamUI           mMayaCam;                   // 3d camera
    
    BlobModuleRef           mModule;                    // the effects
    
    params::InterfaceGlRef  mParams;
    
    float                   mFadeIn, mFadeOut;
    float                   mSpeed, mRadius;
};


void BlobApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 1200, 800 );
}


void BlobApp::setup()
{
    addAssetDirectory( "../../../../assets/" );
    
    loadFixtures( "fixtures_001.csv" );                                             // load CSV fixtures file

    mFixtureMesh    = loadObj( "sphere.obj" );                                      // load Fixture mesh
    mVenueMesh      = loadObj( "venue.obj" );                                       // load Venue mesh
    
    mFadeIn         = 0.1f;
    mFadeOut        = 0.1f;
    mSpeed          = 1.0f;
    mRadius         = 6.0f;
    
    mModule         = BlobModule::create( Vec3f(0.0f, 5.0f, 0.0f), 12 );            // create module

    mParams         = params::InterfaceGl::create( "Params", Vec2i( 200, 240 ) );   // Gui
    
	mParams->addParam( "Fade IN",   &mFadeIn    , "min=0.001 max=1.0 step=0.001" );
    mParams->addParam( "Fade OUT",  &mFadeOut   , "min=0.001 max=1.0 step=0.001" );
	mParams->addSeparator();
	
    mParams->addParam( "Speed",     &mSpeed     , "min=0.001 max=10.0 step=0.001" );
    mParams->addParam( "Radius",    &mRadius    , "min=0.1 max=15.0 step=0.1" );
    
    ci::CameraPersp initialCam;                                                     // Initialise camera
    initialCam.setPerspective( 45.0f, ci::app::getWindowAspectRatio(), 0.1, 3000 );
    mMayaCam        = MayaCamUI( initialCam );
}


void BlobApp::update()
{
    mModule->update( mFixtures, mSpeed, mRadius );
    
    for( size_t k=0; k < mFixtures.size(); k++ )        // update fixtures brightness
        mFixtures[k]->update( mFadeIn, mFadeOut );
}


void BlobApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::enableAlphaBlending();
    
    gl::setMatrices( mMayaCam.getCamera() );                   // set camera matrices
    
    renderGrid();
    
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


void BlobApp::loadFixtures( string fileName, bool flipZ )
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


gl::VboMeshRef BlobApp::loadObj( string fileName )
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


void BlobApp::renderGrid()
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


void BlobApp::keyDown( KeyEvent event )
{
    char c = event.getChar();
    
    if ( c == 'f' )
        setFullScreen( !isFullScreen() );
}


void BlobApp::mouseDown( ci::app::MouseEvent event )
{
    if( event.isAltDown() )
        mMayaCam.mouseDown( event.getPos() );
}


void BlobApp::mouseDrag( ci::app::MouseEvent event )
{
    if( event.isAltDown() )
        mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}


void BlobApp::resize()
{
    ci::CameraPersp cam = mMayaCam.getCamera();
    cam.setPerspective( 45.0f, ci::app::getWindowAspectRatio(), 0.1, 3000 );
    mMayaCam.setCurrentCam( cam );
}


CINDER_APP_NATIVE( BlobApp, RendererGl )
