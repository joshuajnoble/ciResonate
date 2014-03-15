#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

#include "Scene.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class BlobApp : public AppNative {
public:
	void prepareSettings( Settings *settings );
	void setup();
    void keyDown( KeyEvent event );
	void update();
	void draw();
    
    void importFixturesFile( fs::path filePath, bool flipZ );
    
    SceneRef                mScene;         // scene object is responsible for updating and rendering the scene
    std::shared_ptr<float>  mTmpValues;     // temp values written by each effect

};


void BlobApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 1200, 800 );
}


void BlobApp::setup()
{
    addAssetDirectory( "../../../../assets/" );
    
    // initialised the scene
    mScene = Scene::create( getAssetPath( "../assets/fixtures_2.csv" ), true,
                            getAssetPath( "../assets/platonic.obj" ), getAssetPath( "../assets/venue.obj" ) );
    
    mScene->setGridSize( 14 );
    mScene->setVenueColor( Color::gray(0.8f) );
    
    mTmpValues  = std::shared_ptr<float>( new float[ mScene->getFixturesN() ] );
    for( size_t k=0; k < mScene->getFixturesN(); k++ )
        mTmpValues.get()[k] = 0.0f;
}


void BlobApp::keyDown( KeyEvent event )
{
    char c = event.getChar();
    if ( c == 'f' )
        setFullScreen( !isFullScreen() );
}


void BlobApp::update()
{
    // update values
    for( size_t k=0; k < mScene->getFixturesN(); k++ )
        mTmpValues.get()[k] = randFloat();//0.0f;
    
    // update modules
//    for( size_t k=0; k < mModules.size(); k++ )
//        mModules[k]->update( mTmpValues, mScene->getFixturesN() );
    
    // update scene
    if ( mScene )
    	mScene->update( mTmpValues, 0.1f, 0.1f );
}


void BlobApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::enableAlphaBlending();
    
    mScene->begin();
//    for( size_t k=0; k < mModules.size(); k++ )
//    {
//        if ( mModules[k]->isPlaying() )
//        {
//            mModules[k]->render(height);
//            height += 0.2f;
//        }
//    }
    mScene->end();
}


CINDER_APP_NATIVE( BlobApp, RendererGl )
