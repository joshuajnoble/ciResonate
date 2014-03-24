
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include <boost/circular_buffer.hpp>
#include "ciXtractReceiver.h"

using namespace ci;
using namespace ci::app;
using namespace std;

/*
    **************************
    *** LibXtract Features ***
    **************************

    XTRACT_SPECTRAL_CENTROID,
    XTRACT_VARIANCE,
    XTRACT_HARMONIC_SPECTRUM
    XTRACT_MEAN,
    XTRACT_VARIANCE,
    XTRACT_STANDARD_DEVIATION,
    XTRACT_AVERAGE_DEVIATION,
    XTRACT_SKEWNESS,
    XTRACT_KURTOSIS,
    XTRACT_SPECTRAL_MEAN,
    XTRACT_SPECTRAL_VARIANCE,
    XTRACT_SPECTRAL_STANDARD_DEVIATION,
    XTRACT_SPECTRAL_SKEWNESS,
    XTRACT_SPECTRAL_KURTOSIS,
    XTRACT_SPECTRAL_CENTROID,
    XTRACT_IRREGULARITY_K,
    XTRACT_IRREGULARITY_J,
    XTRACT_TRISTIMULUS_1,
    XTRACT_SMOOTHNESS,
    XTRACT_SPREAD,
    XTRACT_ZCR,
    XTRACT_ROLLOFF,
    XTRACT_LOUDNESS,
    XTRACT_FLATNESS,
    XTRACT_FLATNESS_DB,
    XTRACT_TONALITY,
    XTRACT_CREST,
    XTRACT_NOISINESS,
    XTRACT_RMS_AMPLITUDE,
    XTRACT_SPECTRAL_INHARMONICITY,
    XTRACT_POWER,
    XTRACT_ODD_EVEN_RATIO,
    XTRACT_SHARPNESS,
    XTRACT_SPECTRAL_SLOPE,
    XTRACT_LOWEST_VALUE,
    XTRACT_HIGHEST_VALUE,
    XTRACT_SUM,
    XTRACT_NONZERO_COUNT,
    XTRACT_HPS,
    XTRACT_F0,
    XTRACT_FAILSAFE_F0,
    XTRACT_WAVELET_F0,
    XTRACT_AUTOCORRELATION,
    XTRACT_BARK_COEFFICIENTS,
    XTRACT_PEAK_SPECTRUM,
    XTRACT_SPECTRUM,
    XTRACT_MFCC,
    XTRACT_HARMONIC_SPECTRUM,
    XTRACT_SUBBANDS
*/


class SoundAnalysisApp : public AppNative {
    
public:
    
    void prepareSettings(Settings *settings);
	void setup();
	void mouseDown( MouseEvent event );
	void update();
	void draw();
    
    void initGui();
    void drawData( FeatureDataRef feature, Rectf rect );
    
    void selectFeature( string name );
    
public:
    
    ciXtractReceiverRef             mXtract;
    FeatureDataRef                  mSelectedFeature;
    float                           mDataGain;
    float                           mDataOffset;
    float                           mDataDamping;
    float                           mDataSpread;
    float                           mDataSpreadOffset;
    vector<string>                  mAvailableFeatures;
    int                             mActiveFeature;
    bool                            mDataIsLog;
    boost::circular_buffer<float>   mDataBuffer;
    
    params::InterfaceGlRef          mParams;
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
    mActiveFeature      = 0;
    mDataIsLog          = false;
    
    mDataBuffer.resize( 100 );
    
    mAvailableFeatures.push_back( "XTRACT_SPECTRUM" );
    mAvailableFeatures.push_back( "XTRACT_BARK_COEFFICIENTS" );
    mAvailableFeatures.push_back( "XTRACT_MEAN" );
    mAvailableFeatures.push_back( "XTRACT_SPREAD" );
    
    initGui();
    
    // setup receiver
    mXtract = ciXtractReceiver::create();
}


void SoundAnalysisApp::mouseDown( MouseEvent event )
{
}


void SoundAnalysisApp::update()
{
    // change feature
    if ( !mSelectedFeature || mSelectedFeature->getName() != mAvailableFeatures[mActiveFeature] )
        selectFeature( mAvailableFeatures[mActiveFeature] );
    
    // set feature properties
    mSelectedFeature->setLog( mDataIsLog );
    mSelectedFeature->setGain( mDataGain );
    mSelectedFeature->setOffset( mDataOffset );
    mSelectedFeature->setDamping( mDataDamping );
 
    // process audio analysis
    mXtract->update();
}


void SoundAnalysisApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::enableAlphaBlending();
    
    drawData( mSelectedFeature, Rectf( 16, 250, 236, 320 ) );
    
    mParams->draw();
}


void SoundAnalysisApp::initGui()
{
    mParams = params::InterfaceGl::create( "Settings", Vec2f( 220, 200 ) );
    
    mParams->addParam( "Data Gain",     &mDataGain,         "min=0.0 max=100.0 step=0.1" );
    mParams->addParam( "Data Offset",   &mDataOffset,       "min=-1.0 max=1.0 step=0.01" );
    mParams->addParam( "Data Damping",  &mDataDamping,      "min=0.0 max=0.99 step=0.01" );
    mParams->addParam( "Use log",       &mDataIsLog );
    mParams->addParam( "Spread",        &mDataSpread,       "min=0.0 max=1.0 step=0.01" );
    mParams->addParam( "Spread Offset", &mDataSpreadOffset, "min=0.0 max=1.0 step=0.01" );
    mParams->addParam( "Feature",       mAvailableFeatures, &mActiveFeature );
}


void SoundAnalysisApp::drawData( FeatureDataRef feature, Rectf rect )
{   
    glPushMatrix();
    
    gl::drawString( feature->getName(), rect.getUpperLeft(), Color::white() );
    
    rect.y1 += 10;
    
    gl::color( ColorA( 1.0f, 1.0f, 1.0f, 0.1f ) );
    
    gl::drawSolidRect( rect );
    
    gl::translate( rect.getUpperLeft() );
    
    
    std::shared_ptr<float>  data  = feature->getData();
    float                   min   = feature->getMin();
    float                   max   = feature->getMax();
    float                   h     = rect.getHeight();
    float                   val, barY, step;
	PolyLine<Vec2f>         buffLine;
    
    gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
    
    // render Vector feature
    if ( feature->getSize() > 1 )
    {
        step = rect.getWidth() / feature->getSize();
        
        for( int i = 0; i < feature->getSize(); i++ )
        {
            val     = ( data.get()[i] - min ) / ( max - min );
            val     = math<float>::clamp( val, 0.0f, 1.0f );
            barY    = h * val;
            buffLine.push_back( Vec2f( i * step, h-barY ) );
        }
    }
    
    // render Scalar feature + buffered data
    else if ( feature->getSize() == 1 )
    {
        // buffer the latest value
        mDataBuffer.push_back( data.get()[0] );
        
        // draw latest value as bar
        glBegin( GL_QUADS );
        glVertex2f( 0,  h );        glVertex2f( 3,  h );
        glVertex2f( 3,  h-val );    glVertex2f( 0,  h-val );
        glEnd();
        
        step    = rect.getWidth() / ( (float)mDataBuffer.size() - 1 );
        val     = ( data.get()[0] - min ) / ( max - min );
        val     = h * math<float>::clamp( val, 0.0f, 1.0f );
        
        
        
        for( int i = 0; i < mDataBuffer.size(); i++ )
        {
            val     = ( mDataBuffer[ mDataBuffer.size() - i - 1 ] - min ) / ( max - min );
            val     = math<float>::clamp( val, 0.0f, 1.0f );
            barY    = h * val;
            buffLine.push_back( Vec2f( i * step, h-barY ) );
        }
    }
    
    // render data
	if ( buffLine.size() > 0 )
	    gl::draw( buffLine );
    
    gl::popMatrices();
}


void SoundAnalysisApp::selectFeature( string name )
{
	mSelectedFeature = mXtract->getFeatureData( name );

    // reset buffer, only used for scalar features
    for( auto k=0; k < mDataBuffer.size(); k++ )
        mDataBuffer[k] = 0.0f;
}


CINDER_APP_NATIVE( SoundAnalysisApp, RendererGl )

