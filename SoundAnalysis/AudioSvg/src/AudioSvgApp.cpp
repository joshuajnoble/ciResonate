
#include "cinder/app/AppNative.h"

#include "cinder/svg/Svg.h"
#include "cinder/svg/SvgGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include "cinder/MayaCamUI.h"

#include "ciXtractReceiver.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class AudioSvgApp : public AppBasic {

public:
	
	void prepareSettings( Settings *settings );
	
	void setup();
	
    void keyDown( KeyEvent event );
    void mouseDown( MouseEvent event );
    void mouseDrag( MouseEvent event );
    void mouseWheel( MouseEvent event );
    void fileDrop( FileDropEvent event );
    
	void update();
	void draw();
    
    void loadShader();
    
    void loadShape( fs::path filepath );

    void createVbo();
    
    void initGui();
    
    
public:
    
	svg::DocRef				mDoc;
    Shape2d                 mShape;
    gl::VboMesh             mVbo;
    TriMesh2d               mMesh;
    
    params::InterfaceGlRef  mParams;
    
    float                   mLineWidth;

    ciXtractReceiverRef     mXtract;			// ciXtract receiver instance
    FeatureDataRef          mData;				// the data received
	Surface32f				mDataSurf;			// data is stored in Surface
	gl::Texture				mDataTex;			// use a Texture to pass the data to the shader

    float                   mDataGain;
    float                   mDataOffset;
    float                   mDataDamping;
    float                   mScale;
    Vec2f                   mOffset;
    float                   mDataSpread;
    float                   mDataSpreadOffset;
    Vec2f                   mPrevMousePos;
    gl::GlslProgRef         mShader;    
    fs::path                mSvgFilepath;
	ColorA					mSvgColor;
};


void AudioSvgApp::prepareSettings(Settings *settings)
{
	settings->setTitle("ciXtract OSC Receiver");
	settings->setWindowSize( 1280, 720 );
}


void AudioSvgApp::setup()
{
    mLineWidth          = 1.0f;
    mScale              = 1.0f;
    mOffset             = Vec2f::zero();
    
    mDataGain           = 1.0f;
    mDataOffset         = 0.0f;
    mDataDamping        = 0.85f;
    
    mDataSpread         = 1.0f;
    mDataSpreadOffset   = 0.0f;
    
	mSvgColor			= ColorA::white();

    loadShape( getAssetPath( "default.svg" ) );
    
    initGui();
    
    mXtract = ciXtractReceiver::create();
	mData   = mXtract->getFeatureData( "XTRACT_SPECTRUM" );
    mData->setLog( true );
    
    loadShader();

	mDataSurf	= Surface32f( 32, 32, false );	// we can store up to 1024 values(32x32)
	mDataTex	= gl::Texture( 32, 32 );
}


void AudioSvgApp::keyDown( KeyEvent event )
{
    char c = event.getChar();

    if ( c == 'f' )
        setFullScreen( !isFullScreen() );

    else if ( c == 'r' )
        loadShader();
}

void AudioSvgApp::mouseDown( MouseEvent event )
{
    mPrevMousePos = event.getPos();
}


void AudioSvgApp::mouseDrag( MouseEvent event )
{
    mOffset         += event.getPos() - mPrevMousePos;
    mPrevMousePos   = event.getPos();
}


void AudioSvgApp::mouseWheel( MouseEvent event )
{
    mScale += event.getWheelIncrement() * 0.1f;
}


void AudioSvgApp::fileDrop( FileDropEvent event )
{
    fs::path filepath = event.getFile(0);

    if ( filepath.extension() == ".svg" )
		loadShape( filepath );
}


void AudioSvgApp::update()
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


void AudioSvgApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    gl::enableAlphaBlending();

	if ( mData && mDataTex )
	{
		gl::pushMatrices();

		mShader->bind();
		mDataTex.enableAndBind();
		mShader->uniform( "dataTex",		0 );
		mShader->uniform( "texWidth",		(float)mDataTex.getWidth() );
		mShader->uniform( "texHeight",		(float)mDataTex.getHeight() );
		mShader->uniform( "soundDataSize",  (float)mData->getSize() );
		mShader->uniform( "spread",         mDataSpread );
		mShader->uniform( "spreadOffset",   mDataSpreadOffset );
		mShader->uniform( "time",           (float)getElapsedSeconds() );
		mShader->uniform( "tintColor",		mSvgColor );

		gl::scale( mScale * Vec2f::one() );
		gl::translate( mOffset );
    
		gl::draw( mVbo );

		mShader->unbind();
		mDataTex.unbind();
	
		gl::popMatrices();
	}

	gl::color( Color::white() );
	
	ciXtractReceiver::drawData( mData, Rectf( 15, getWindowHeight() - 150, 200, getWindowHeight() - 15 ) );
	
	gl::draw( mDataSurf );

    mParams->draw();
}


void AudioSvgApp::loadShader()
{
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


void AudioSvgApp::loadShape( fs::path filepath )
{
    double t = getElapsedSeconds();
    
    try {
        if ( filepath != mSvgFilepath )
        {
            mDoc            = svg::Doc::create( filepath );
            mShape          = mDoc->getShape();
            mSvgFilepath    = filepath;
            
            console() << "SVG loaded in " << ( getElapsedSeconds() - t ) << " seconds" << endl;
        }
    }
    
	catch( ... ) {
        console() << "cannot load svg!" << endl;
	}

	createVbo();
}


void AudioSvgApp::createVbo()
{
    Rectf               rect;
    vector<Vec2f>       texCoords;
    vector<Vec3f>       vertices;
    vector<ColorA>      colors;
    vector<uint32_t>    indices;
        
    Vec2f p, tex;
        
	rect    = mShape.calcPreciseBoundingBox();
	int		totalPoints = 0;
	int		c = 0;

	for( size_t k=0; k < mShape.getNumContours(); k++ )
        totalPoints += mShape.getContours()[k].getNumPoints();

    for( size_t k=0; k < mShape.getNumContours(); k++ )
	{
        for( auto i=0; i < mShape.getContours()[k].getNumPoints(); i++ )
        {
			if ( i == mShape.getContours()[k].getNumPoints() - 1 && i % 2 == 0 )
				continue;

			p		= mShape.getContours()[k].getPoints()[i];
			tex.x	= (float)c / (float)totalPoints;
			c++;

            indices.push_back( vertices.size() );
			vertices.push_back( Vec3f( p.x, p.y, 0.0f ) );
			colors.push_back( ColorA::white() );
			texCoords.push_back( tex );
        }
	}
	
	mOffset = getWindowCenter() - rect.getCenter();

    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    layout.setStaticTexCoords2d();
	layout.setStaticColorsRGBA();

	mVbo = gl::VboMesh( vertices.size(), indices.size(), layout, GL_LINES );

	mVbo.bufferPositions( vertices );
	mVbo.bufferIndices( indices );
	mVbo.bufferColorsRGBA( colors );
	mVbo.bufferTexCoords2d( 0, texCoords );
}


void AudioSvgApp::initGui()
{
    mParams = params::InterfaceGl::create( "Settings", Vec2f( 200, 250 ) );
    mParams->addParam( "Line width",	&mLineWidth );
    mParams->addParam( "Scale",			&mScale, "min=0.0 max=10.0 step=0.1" );
    mParams->addParam( "Offset X",		&mOffset.x, "min=-1000.0 max=1000.0 step=1" );
    mParams->addParam( "Offset Y",		&mOffset.y, "min=-1000.0 max=1000.0 step=1" );
	mParams->addParam( "SVG color",		&mSvgColor );
    
    mParams->addSeparator();    
    mParams->addParam( "Data Gain",     &mDataGain,         "min=0.0 max=25.0 step=0.1" );
    mParams->addParam( "Data Offset",   &mDataOffset,       "min=-1.0 max=1.0 step=0.01" );
    mParams->addParam( "Data Damping",  &mDataDamping,      "min=0.0 max=0.99 step=0.01" );
    mParams->addParam( "Spread",        &mDataSpread,       "min=0.0 max=1.0 step=0.01" );
    mParams->addParam( "Spread Offset", &mDataSpreadOffset, "min=0.0 max=1.0 step=0.01" );
}


CINDER_APP_BASIC( AudioSvgApp, RendererGl )

