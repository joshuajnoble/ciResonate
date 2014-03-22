/*
 *  FluidsModule.h
 *
 *  Created by Andrea Cuius
 *  www.nocte.co.uk
 *
 */

#pragma once

#include "cinder/gl/Fbo.h"
#include "cinderfx/Fluid2D.h"
#include "Fixture.h"


class FluidsModule;
typedef std::shared_ptr<FluidsModule>	FluidsModuleRef;


class FluidsModule {
	
public:
    
    static FluidsModuleRef create( ci::Rectf rect )
    {
        return FluidsModuleRef( new FluidsModule(rect) );
    }
    
    ~FluidsModule() {}
    
    void update( std::vector<FixtureRef> fixtures, float dissipation, float viscosity );
	
	void render();
    
    bool containsPoint( ci::Vec2f pos2f );
    
    void injectRandomFluid( float randomness, float force, float density, int spawns );
    
    void createRandomSources( int n = 10 );
    
protected:
    
	FluidsModule( ci::Rectf rect );
    
    
private:
    
    struct FluidSource {
        ci::Vec2f   pos;
        ci::Vec2f   dir;
    };
    
	ci::gl::Texture             mTex;
	cinderfx::Fluid2D           mFluid2D;
    ci::Rectf                   mFluidRect;
    ci::Channel32f              mFluidChannel;
    std::vector<FluidSource>    mSources;
    
};
