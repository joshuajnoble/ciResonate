/*
 *  SpotlightModule.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#include "Fixture.h"
#include "SpotlightModule.h"

using namespace ci;
using namespace ci::app;
using namespace std;


SpotlightModule::SpotlightModule( Vec3f pos, float radius )
{    
    mPos    = pos;
    mRadius = radius;
}


void SpotlightModule::update( vector<FixtureRef> fixtures, Vec3f pos, float radius )
{
    mPos    = pos;
    mRadius = radius;
    
    float val, dist;
    
    for ( size_t k=0; k < fixtures.size(); k++ )
    {
        dist = fixtures[k]->getPos2f().distance( mPos.xz() );

        if ( dist < mRadius )
            val = 1.0f - dist / mRadius;
        else
            val = 0.0f;
        
        fixtures[k]->setValue( val );
    }
}


void SpotlightModule::render()
{
    gl::color( Color( 0.0f, 1.0f, 1.0f ) );
    gl::drawSphere( mPos, 0.2f );
    
    Vec3f 	vert    = Vec3f( 0.0f, 0.0f, 0.0f );
    int     nPoints = 12;
    float   angle;
    
    glBegin(GL_LINE_LOOP);
    
    for( size_t k=0; k < nPoints; k++ )
    {
        angle =  6.28f * (float)k / (float)nPoints;
        
        vert.x = mRadius * cos( angle );
        vert.z = mRadius * sin( angle );

        gl::vertex( mPos + vert );
    }
    glEnd();
    
	gl::color( Color::white() );
}
