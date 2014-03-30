
#include "cinder/app/AppNative.h"

#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include "cinder/ObjLoader.h"
#include "cinder/MayaCamUI.h"

#include "ciXtractReceiver.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class AudioObjApp : public AppBasic {

public:
	
	void prepareSettings( Settings *settings );
	
	void setup();
	
    void keyDown( KeyEvent event );
    void mouseDown( MouseEvent event );
    void mouseDrag( MouseEvent event );
    void resize();
    void fileDrop( FileDropEvent event );
    
	void update();
	void draw();
    
    void loadShader();
    
    void loadObject( fs::path filepath );

    void initGui();
    
    
public:
    
    gl::VboMesh             mVbo;
    
    params::InterfaceGlRef  mParams;

    ciXtractReceiverRef     mXtract;			// ciXtract receiver instance
    FeatureDataRef          mFeature;				// the data received
	Surface32f				mFeatureSurf;			// data is stored in Surface
	gl::Texture				mFeatureTex;			// use a Texture to pass the data to the shader

    float                   mFeatureGain;
    float                   mFeatureOffset;
    float                   mFeatureDamping;
    float                   mFeatureSpread;
    float                   mFeatureSpreadOffset;
    ColorA                  mObjColor;
    gl::GlslProgRef         mShader;    
    MayaCamUI               mMayaCam;
    bool                    mRenderWireframe;
    
};


void AudioObjApp::prepareSettings(Settings *settings)
{
	settings->setTitle("ciXtract OSC Receiver");
	settings->setWindowSize( 1280, 720 );
}


void AudioObjApp::setup()
{
    mFeatureGain           = 1.0f;
    mFeatureOffset         = 0.0f;
    mFeatureDamping        = 0.85f;
    
    mFeatureSpread         = 1.0f;
    mFeatureSpreadOffset   = 0.0f;
    
    mObjColor           = ColorA( 0.0f, 1.0f, 1.0f, 1.0f );
    mRenderWireframe    = true;
    
    loadObject( getAssetPath( "cube.obj" ) );
    
    initGui();
    
    mXtract     = ciXtractReceiver::create();
	mFeature    = mXtract->getFeatureData( "XTRACT_SPECTRUM" );
    mFeature->setLog( true );
    
    loadShader();
    
	CameraPersp initialCam;
	initialCam.setPerspective( 45.0f, getWindowAspectRatio(), 0.1, 1000 );
	mMayaCam.setCurrentCam( initialCam );

	mFeatureSurf	= Surface32f( 32, 32, false );	// we can store up to 1024 values(32x32)
	mFeatureTex	= gl::Texture( 32, 32 );
}


void AudioObjApp::keyDown( KeyEvent event )
{
    char c = event.getChar();

    if ( c == 'f' )
        setFullScreen( !isFullScreen() );

    else if ( c == 'r' )
        loadShader();
    
    else if ( c == '1' )
        mFeature = mXtract->getFeatureData( "XTRACT_SPECTRUM" );
    
    else if ( c == '2' )
        mFeature = mXtract->getFeatureData( "XTRACT_BARK_COEFFICIENTS" );
}

void AudioObjApp::mouseDown( MouseEvent event )
{
    if( event.isAltDown() )
		mMayaCam.mouseDown( event.getPos() );
}


void AudioObjApp::mouseDrag( MouseEvent event )
{
    if( event.isAltDown() )
		mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}


void AudioObjApp::resize()
{
	CameraPersp cam;
    cam.setPerspective( 45.0f, getWindowAspectRatio(), 0.1, 10000 );
	mMayaCam.setCurrentCam( cam );

}


void AudioObjApp::fileDrop( FileDropEvent event )
{
    fs::path filepath = event.getFile(0);

    if ( filepath.extension() == ".obj" )
    {
        loadObject( filepath );
    }
}


void AudioObjApp::update()
{
	if ( mFeature )
	{
		mFeature->setDamping( mFeatureDamping );
		mFeature->setGain( mFeatureGain );
		mFeature->setOffset( mFeatureOffset );
	}

    mXtract->update();

	if ( mFeature )
	{
		// update Surface
		int x, y;
		for( int k=0; k < mFeature->getSize(); k++ )
		{
			x = k % mFeatureSurf.getWidth();
			y = k / mFeatureSurf.getWidth();
			mFeatureSurf.setPixel( Vec2i(x, y), Color::gray( mFeature->getDataValue(k) ) );
		}
		mFeatureTex = gl::Texture( mFeatureSurf );
	}
}


void AudioObjApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    gl::enableAlphaBlending();
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    gl::pushMatrices();

	gl::setMatrices( mMayaCam.getCamera() );
	
	if ( mFeature && mFeatureTex )
	{
		mShader->bind();
		mFeatureTex.enableAndBind();
		mShader->uniform( "dataTex",		0 );
		mShader->uniform( "texWidth",		(float)mFeatureTex.getWidth() );
		mShader->uniform( "texHeight",		(float)mFeatureTex.getHeight() );
		mShader->uniform( "soundDataSize",  (float)mFeature->getSize() );
		mShader->uniform( "spread",         mFeatureSpread );
		mShader->uniform( "spreadOffset",   mFeatureSpreadOffset );
        mShader->uniform( "time",           (float)getElapsedSeconds() );
		mShader->uniform( "tintColor",      mObjColor );
	}
    
    if ( mRenderWireframe )
        gl::enableWireframe();
    
	gl::color( Color(1.0f, 0.0f, 0.0f ) );
    
	if ( mVbo )
	    gl::draw( mVbo );

    if ( mRenderWireframe )
        gl::disableWireframe();
    
	mShader->unbind();
	mFeatureTex.unbind();

	gl::color( Color::white() );
//	gl::drawCoordinateFrame();
  
	gl::popMatrices();
    
    gl::disableDepthRead();
    gl::disableDepthWrite();
    
	gl::setMatricesWindow( getWindowSize() );

	ciXtractReceiver::drawData( mFeature, Rectf( 15, getWindowHeight() - 150, 255, getWindowHeight() - 35 ) );
	
	gl::draw( mFeatureSurf );

    mParams->draw();
}


void AudioObjApp::loadShader()
{
    console() << getAssetPath( "shaders/passThru.vert" ) << endl;
    
    try {
		mShader = gl::GlslProg::create( loadAsset( "shaders/passThru.vert" ), loadAsset( "shaders/sound.frag" ) );
        console() << "Shader loaded " << getElapsedSeconds() << endl;
	}
	catch( gl::GlslProgCompileExc &exc ) {
		console() << "Shader compile error: " << endl;
		console() << exc.what();
	}
	catch( ... ) {
		console() << "Unable to load shader" << endl;
	}
}


void AudioObjApp::loadObject( fs::path filepath )
{
    try {
		ObjLoader loader( DataSourcePath::create( filepath.string() ) );
		TriMesh mesh;
		loader.load( &mesh );
		mVbo = gl::VboMesh( mesh );
	}    
	catch( ... ) {
		console() << "cannot load file: " << filepath.generic_string() << endl;
		return;
	}
}


void AudioObjApp::initGui()
{
    mParams = params::InterfaceGl::create( "Settings", Vec2f( 200, 250 ) );
    mParams->addParam( "Obj color",     &mObjColor );
    mParams->addParam( "Wireframe",     &mRenderWireframe );
    mParams->addParam( "Data Gain",     &mFeatureGain,         "min=0.0 max=25.0 step=0.1" );
    mParams->addParam( "Data Offset",   &mFeatureOffset,       "min=-1.0 max=1.0 step=0.01" );
    mParams->addParam( "Data Damping",  &mFeatureDamping,      "min=0.0 max=0.99 step=0.01" );
    mParams->addParam( "Spread",        &mFeatureSpread,       "min=0.0 max=1.0 step=0.01" );
    mParams->addParam( "Spread Offset", &mFeatureSpreadOffset, "min=0.0 max=1.0 step=0.01" );
}


CINDER_APP_BASIC( AudioObjApp, RendererGl )

