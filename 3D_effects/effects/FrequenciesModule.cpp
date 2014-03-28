/*
 *  FrequenciesModule.cpp
 *
 *  Created by Andrea Cuius
 *
 *  www.nocte.co.uk
 *
 */

#include "FrequenciesModule.h"
#include "ciXtractReceiver.h"

using namespace ci;
using namespace ci::app;
using namespace std;


void FrequenciesModule::update( std::vector<FixtureRef> fixtures, FeatureDataRef feature, float spread, float offset, float brightness )
{
    if ( !feature )
        return;
    
    int     binN;
    float   val;
    int     dataSize    = feature->getSize() * spread;                  // only take a portion of the data in %
    int     dataOffset  = feature->getSize() * offset;                  // bin offset
    float   step        = (float)dataSize / (float)fixtures.size();     // fixture bins are equidistant
    
    // update values
    for( size_t k=0; k < fixtures.size(); k++ )
    {
        binN    = k * step + dataOffset;
        val     = brightness * feature->getDataValue( binN );
        
        fixtures[k]->setValue( val );
    }
}


void FrequenciesModule::render()
{
}

