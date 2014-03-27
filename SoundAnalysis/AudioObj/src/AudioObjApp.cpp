
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
    FeatureDataRef          mData;				// the data received
	Surface32f				mDataSurf;			// data is stored in Surface
	gl::Texture				mDataTex;			// use a Texture to pass the data to the shader

    float                   mDataGain;
    float                   mDataOffset;
    float                   mDataDamping;
    float                   mDataSpread;
    float                   mDataSpreadOffset;
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
    mDataGain           = 1.0f;
    mDataOffset         = 0.0f;
    mDataDamping        = 0.85f;
    
    mDataSpread         = 1.0f;
    mDataSpreadOffset   = 0.0f;
    
    mObjColor           = ColorA( 0.0f, 1.0f, 1.0f, 1.0f );
    mRenderWireframe    = true;
    
    loadObject( getAssetPath( "cube.obj" ) );
    
    initGui();
    
    mXtract = ciXtractReceiver::create();
//	mData   = mXtract->getFeatureData( "XTRACT_BARK_COEFFICIENTS" );
	mData   = mXtract->getFeatureData( "XTRACT_SPECTRUM" );
    mData->setLog( true );
    
    loadShader();
    
	CameraPersp initialCam;
	initialCam.setPerspective( 45.0f, getWindowAspectRatio(), 0.1, 1000 );
	mMayaCam.setCurrentCam( initialCam );

	mDataSurf	= Surface32f( 32, 32, false );	// we can store up to 1024 values(32x32)
	mDataTex	= gl::Texture( 32, 32 );
}


void AudioObjApp::keyDown( KeyEvent event )
{
    char c = event.getChar();

    if ( c == 'f' )
        setFullScreen( !isFullScreen() );

    else if ( c == 'r' )
        loadShader();
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
	if ( mData )
	{
		mData->setDamping( mDataDamping );
		mData->setGain( mDataGain );
		mData->setOffset( mDataOffset );
	}

    mXtract->update();

	if ( mData )
	{
		// update Surface
		int x, y;
		for( int k=0; k < mData->getSize(); k++ )
		{
			x = k % mDataSurf.getWidth();
			y = k / mDataSurf.getWidth();
			mDataSurf.setPixel( Vec2i(x, y), Color::gray( mData->getDataValue(k) ) );
		}
		mDataTex = gl::Texture( mDataSurf );
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
	
	if ( mData && mDataTex )
	{
		mShader->bind();
		mDataTex.enableAndBind();
		mShader->uniform( "dataTex",		0 );
		mShader->uniform( "texWidth",		(float)mDataTex.getWidth() );
		mShader->uniform( "texHeight",		(float)mDataTex.getHeight() );
		mShader->uniform( "soundDataSize",  (float)mData->getSize() );
		mShader->uniform( "spread",         mDataSpread );
		mShader->uniform( "spreadOffset",   mDataSpreadOffset );
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
	mDataTex.unbind();

	gl::color( Color::white() );
//	gl::drawCoordinateFrame();
  
	gl::popMatrices();
    
    gl::disableDepthRead();
    gl::disableDepthWrite();
    
	gl::setMatricesWindow( getWindowSize() );

	ciXtractReceiver::drawData( mData, Rectf( 15, getWindowHeight() - 150, 200, getWindowHeight() - 15 ) );
	
	gl::draw( mDataSurf );

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
    mParams->addParam( "Data Gain",     &mDataGain,         "min=0.0 max=25.0 step=0.1" );
    mParams->addParam( "Data Offset",   &mDataOffset,       "min=-1.0 max=1.0 step=0.01" );
    mParams->addParam( "Data Damping",  &mDataDamping,      "min=0.0 max=0.99 step=0.01" );
    mParams->addParam( "Spread",        &mDataSpread,       "min=0.0 max=1.0 step=0.01" );
    mParams->addParam( "Spread Offset", &mDataSpreadOffset, "min=0.0 max=1.0 step=0.01" );
}


CINDER_APP_BASIC( AudioObjApp, RendererGl )

