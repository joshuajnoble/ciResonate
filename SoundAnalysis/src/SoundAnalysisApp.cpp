
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "ciXtractReceiver.h"
#include "cinder/params/Params.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class SoundAnalysisApp : public AppNative {
    
public:
    
    //    enum AvailableFeatures {
    //        "XTRACT_SPECTRUM",
    //        "XTRACT_BARK_COEFFICIENTS"
    //    };
    
    void prepareSettings(Settings *settings);
	void setup();
	void mouseDown( MouseEvent event );
	void update();
	void draw();
    
    void initGui();
    void drawData( FeatureDataRef feature, Rectf rect );
    
public:
    
    ciXtractReceiverRef     mXtract;
    FeatureDataRef          mData;
    float                   mDataGain;
    float                   mDataOffset;
    float                   mDataDamping;
    float                   mDataSpread;
    float                   mDataSpreadOffset;
    
    params::InterfaceGlRef  mParams;
};


void SoundAnalysisApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1280, 720 );
}


void SoundAnalysisApp::setup()
{
    mDataGain           = 1.0f;
    mDataOffset         = 0.0f;
    mDataDamping        = 0.85f;
    mDataSpread         = 1.0f;
    mDataSpreadOffset   = 0.0f;
    
    initGui();
    
    mXtract = ciXtractReceiver::create();
	mData   = mXtract->getFeatureData( "XTRACT_SPECTRUM" );
    mData->setLog( true );
}


void SoundAnalysisApp::mouseDown( MouseEvent event )
{
}


void SoundAnalysisApp::update()
{
    mXtract->update();
}


void SoundAnalysisApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::enableAlphaBlending();
    
    drawData( mData, Rectf( 15, 15, 200, 50 ) );
}


void SoundAnalysisApp::initGui()
{
    mParams = params::InterfaceGl::create( "Settings", Vec2f( 200, 250 ) );
    
    mParams->addParam( "Data Gain",     &mDataGain,         "min=0.0 max=25.0 step=0.1" );
    mParams->addParam( "Data Offset",   &mDataOffset,       "min=-1.0 max=1.0 step=0.01" );
    mParams->addParam( "Data Damping",  &mDataDamping,      "min=0.0 max=0.99 step=0.01" );
    mParams->addParam( "Spread",        &mDataSpread,       "min=0.0 max=1.0 step=0.01" );
    mParams->addParam( "Spread Offset", &mDataSpreadOffset, "min=0.0 max=1.0 step=0.01" );
}


void SoundAnalysisApp::drawData( FeatureDataRef feature, Rectf rect )
{
    ColorA bgCol    = ColorA( 1.0f, 1.0f, 1.0f, 0.1f );
    
    ColorA dataCol  = ColorA( 1.0f, rect.y1 / getWindowHeight(), rect.x1 / getWindowWidth(), 1.0f );
    
    glPushMatrix();
    
    gl::drawString( feature->getName(), rect.getUpperLeft(), Color::white() );
    
    rect.y1 += 10;
    
    gl::color( bgCol );
    
    gl::drawSolidRect( rect );
    
    gl::translate( rect.getUpperLeft() );
    
    glBegin( GL_QUADS );
    
    std::shared_ptr<float>  data  = feature->getData();
    int                     dataN = feature->getSize();
    float                   min   = feature->getMin();
    float                   max   = feature->getMax();
    
    float                   step  = rect.getWidth() / dataN;
    float                   h     = rect.getHeight();
    float                   val, barY;
    
    gl::color( dataCol );
    
    for( int i = 0; i < dataN; i++ )
    {
        val     = ( data.get()[i] - min ) / ( max - min );
        val     = math<float>::clamp( val, 0.0f, 1.0f );
        barY    = h * val;
        
        glVertex2f( i * step,           h );
        glVertex2f( ( i + 1 ) * step,   h );
        glVertex2f( ( i + 1 ) * step,   h-barY );
        glVertex2f( i * step,           h-barY );
    }
    
    glEnd();
    
    gl::popMatrices();
}


CINDER_APP_NATIVE( SoundAnalysisApp, RendererGl )

