/*
 *  FrequenciesModule.h
 *
 *  Created by Andrea Cuius
 *  www.nocte.co.uk
 *
 */

#include "Fixture.h"

class FrequenciesModule;
typedef std::shared_ptr<FrequenciesModule>  FrequenciesModuleRef;

class FeatureData;
typedef std::shared_ptr<FeatureData>        FeatureDataRef;


class FrequenciesModule {
	
public:
    
    static FrequenciesModuleRef create()
    {
        return FrequenciesModuleRef( new FrequenciesModule() );
    }
    
    ~FrequenciesModule() {}
    
    void update( std::vector<FixtureRef> fixtures, FeatureDataRef feature, float spread, float offset, float brightness );
	
	void render();
    
    bool containsPoint( ci::Vec2f pos2f );
    
    
protected:
    
	FrequenciesModule() {}
    
    
};