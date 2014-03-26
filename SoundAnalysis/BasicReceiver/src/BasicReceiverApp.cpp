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


class BasicReceiverApp : public AppNative {
    
public:
    
    void prepareSettings(Settings *settings);
	void setup();
	void update();
	void draw();
    
    void initGui();
    
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
    
    params::InterfaceGlRef          mParams;
};


void BasicReceiverApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1280, 720 );
}


void BasicReceiverApp::setup()
{
    mDataGain           = 1.0f;
    mDataOffset         = 0.0f;
    mDataDamping        = 0.85f;
    mDataSpread         = 1.0f;
    mDataSpreadOffset   = 0.0f;
    mActiveFeature      = 0;
    mDataIsLog          = false;
    
    mAvailableFeatures.push_back( "XTRACT_SPECTRUM" );
    mAvailableFeatures.push_back( "XTRACT_BARK_COEFFICIENTS" );
    mAvailableFeatures.push_back( "XTRACT_HARMONIC_SPECTRUM" );
    mAvailableFeatures.push_back( "XTRACT_VARIANCE" );
    
    initGui();
    
    // setup receiver
    mXtract = ciXtractReceiver::create();
}


void BasicReceiverApp::update()
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


void BasicReceiverApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::enableAlphaBlending();
    
	Rectf			rect( 16, 250, 236, 320 );
	FeatureDataRef	feature;
	for( size_t k=0; k < mAvailableFeatures.size(); k++ )
	{
		feature = mXtract->getFeatureData( mAvailableFeatures[k] );
		ciXtractReceiver::drawData( feature, rect );
		rect.offset( Vec2f( 0, rect.getHeight() + 15 ) );
	}

    mParams->draw();
}


void BasicReceiverApp::initGui()
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



void BasicReceiverApp::selectFeature( string name )
{
	mSelectedFeature = mXtract->getFeatureData( name );
}


CINDER_APP_NATIVE( BasicReceiverApp, RendererGl )

