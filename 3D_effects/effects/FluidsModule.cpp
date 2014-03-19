/*
 *  FluidsModule.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */


#include "FluidsModule.h"
#include "Fixture.h"


using namespace ci;
using namespace ci::app;
using namespace std;


FluidsModule::FluidsModule( ci::Rectf rect )
{
    mFluid2D.set( 32, 32 );
 	mFluid2D.setDensityDissipation( 0.01f );
	mFluid2D.enableDensity();
	mFluid2D.enableVorticityConfinement();
	mFluid2D.initSimData();
    
    mFluidRect = rect;
    
    createRandomSources();
}


void FluidsModule::update( std::vector<FixtureRef> fixtures, float dissipation, float viscosity )
{
    mFluid2D.setDensityDissipation( dissipation );
    mFluid2D.setVelocityDissipation( dissipation );
    mFluid2D.setDensityViscosity( viscosity );
    mFluid2D.setVelocityViscosity( viscosity );
    
	mFluid2D.step();
    
    mFluidChannel = Channel32f( mFluid2D.resX(), mFluid2D.resY(), mFluid2D.resX() * sizeof(float), 1, const_cast<float*>( mFluid2D.density().data() ) );
    
	if( ! mTex )
		mTex = gl::Texture( mFluidChannel );
    else
		mTex.update( mFluidChannel );
 
    // update fixtures
    FixtureRef  fixture;
    float       val;
    Vec2i       pos;

    float       ratioX = mFluid2D.resX() / mFluidRect.getWidth();
    float       ratioY = mFluid2D.resY() / mFluidRect.getHeight();
    
	for ( size_t k=0; k < fixtures.size(); k++ )
    {
        fixture = fixtures[k];
        pos.x   = ratioX * ( fixture->getPos2f().x - mFluidRect.x1 );
        pos.y   = ratioY * ( fixture->getPos2f().y - mFluidRect.y1 );
        val     = *mFluidChannel.getData( pos );
        
        fixture->setValue( val );
	}
}


void FluidsModule::render()
{
    Vec3f   sourcePos;
    float   height = 5.0f;
    
    gl::color( Color( 0.0f, 1.0f, 1.0f ) );
    
    for( auto k=0; k < mSources.size(); k++ )
    {
        sourcePos = Vec3f(  mFluidRect.x1 + mFluidRect.getWidth() * mSources[k].pos.x / mFluid2D.resX(),
                            height,
                            mFluidRect.y1 + mFluidRect.getHeight() * mSources[k].pos.y / mFluid2D.resY() );

        gl::drawCube( sourcePos, Vec3f::one() * 0.1f );                                                     // draw the source position
        gl::drawLine( sourcePos, sourcePos + Vec3f( mSources[k].dir.x, 0, mSources[k].dir.y ) );            // draw the direction
    }

    mTex.enableAndBind();
    
    gl::color( ColorA( 1.0f, 1.0f, 1.0f, 0.35f ) );
    
    glBegin( GL_QUADS );
    gl::texCoord( Vec2f( 0.0f, 0.0f ) );    gl::vertex( mFluidRect.x1, height, mFluidRect.y1 );
    gl::texCoord( Vec2f( 1.0f, 0.0f ) );    gl::vertex( mFluidRect.x2, height, mFluidRect.y1 );
    gl::texCoord( Vec2f( 1.0f, 1.0f ) );    gl::vertex( mFluidRect.x2, height, mFluidRect.y2 );
    gl::texCoord( Vec2f( 0.0f, 1.0f ) );    gl::vertex( mFluidRect.x1, height, mFluidRect.y2 );
    glEnd();
    
    mTex.disable();
    
    gl::color( Color::white() );
    glBegin( GL_LINE_LOOP );
    gl::vertex( mFluidRect.x1, height, mFluidRect.y1 );
    gl::vertex( mFluidRect.x2, height, mFluidRect.y1 );
    gl::vertex( mFluidRect.x2, height, mFluidRect.y2 );
    gl::vertex( mFluidRect.x1, height, mFluidRect.y2 );
    glEnd();
    
    
}


void FluidsModule::injectRandomFluid( float randomness, float force, float density, int spawns )
{
    float rnd;
    Vec2f vel;
    float velScale = 10.0f * force * mFluid2D.resX();       // the scale is proportioned to the fluid XY size

    for( auto k=0; k < mSources.size(); k++ )               // inject fluid for each source
    {
        rnd = randFloat();
        
        if ( rnd > randomness )                             // add some randomness
            continue;
        
        rnd *= 6.14f;
    
        for( auto i=0; i < spawns; i++ )                    // inject fluid N times
        {
            vel = velScale * Vec2f( cos(rnd), sin(rnd) );   // spread the fluid in all directions
            
            mFluid2D.splatDensity( mSources[k].pos.x, mSources[k].pos.y, density );     // add more fluid
            mFluid2D.splatVelocity( mSources[k].pos.x, mSources[k].pos.y, vel );        // add the velocity(push the fluid)
        }
    }
}

void FluidsModule::createRandomSources( int n )
{
    mSources.clear();       // destory all the current sources
    
    float   angle;
    Vec2f   pos, dir;

    for( auto k=0; k < n; k++ )
    {
        // each sources include position and direction(not used atm) 
        angle   = randFloat( 3.14f );
        pos     = Vec2f( randInt( mFluid2D.resX() * 0.2f, mFluid2D.resX() * 0.8f ),
                        randInt( mFluid2D.resY() * 0.2f, mFluid2D.resY() * 0.8f ) );
        dir     = Vec2f( cos(angle), sin(angle) ).normalized();
        
        mSources.push_back( { pos, dir } );
    }
}