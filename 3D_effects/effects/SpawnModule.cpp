/*
 *  SpawnModule.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#include "SpawnModule.h"
#include "Fixture.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;


SpawnModule::SpawnModule()
{
    createPath( 10, 3, 2 );
}


void SpawnModule::update( vector<FixtureRef> fixtures, float speed )
{
    for( size_t k=0; k < mPaths.size(); )
    {
        mPaths[k].posNorm += 0.1f * speed / mPaths[k].spline.getNumControlPoints();
    
        if ( mPaths[k].posNorm > 1.0f )
            mPaths.erase( mPaths.begin() + k );
        else
            k++;
    }
    
    // update Fixture values
    float val, dist;
    for ( size_t k=0; k < fixtures.size(); k++ )
    {
        val = 0.0f;
        fixtures[k]->setValue( val );                                               // reset value
        
        for( size_t i=0; i < mPaths.size(); i++ )
        {
            dist = mPaths[i].getCurrentPos().distance( fixtures[k]->getPos() );     // calculate the distance between the fixture and the center of the sphere
            dist /= mPaths[i].radius;                                               // normalise distance [0..1]

            if ( dist < 1.0f )
                val += 1.0f - dist;                                                 // the closer the brighter
        }
        
        fixtures[k]->addValue( val );                                               // add value, don't set it so the values for each path can be added together
    }
}


void SpawnModule::render()
{
    int     pathPoints = 100;                                                       // choose how many points to draw the path, more points create smoother paths
    Vec3f   pos;
    
    for( size_t k=0; k < mPaths.size(); k++ )
    {
        ci::gl::color( ColorA( 0.8f, 1.0f, 1.0f, 1.0f ) );                          // render path
        
        glLineWidth( 2.0f );                                                        // the line is a bit thicker than usual
        
        ci::gl::begin( GL_LINE_STRIP );                                             // render as line strip, only send the line vertices, opengl then connects them together
        
        for( int s = 0; s <= pathPoints; ++s )
        {
            float t = (float)s / (float)pathPoints;
            ci::gl::vertex( mPaths[k].spline.getPosition( t ) );
        }
        
        ci::gl::end();                                                              // end line strip
        
        glLineWidth( 1.0f );                                                        // set the line width to the 1.0f, default value
        
        
        pos = mPaths[k].getCurrentPos();
        
        ci::gl::color( ColorA( 1.0f, 1.0f, 1.0f, 0.25f ) );
        ci::gl::enableWireframe();
        ci::gl::drawSphere( pos, mPaths[k].radius );                                // draw outer sphere based on the radius
        ci::gl::disableWireframe();

        ci::gl::drawSphere( pos, 0.1f );                                            // draw position on path
    }
}


void SpawnModule::createPath( int n, int deg, float radius )
{
    if ( n < deg )
        return;
    
    vector<Vec3f> points;
    
    for( int k=0; k < n; k++ )
        points.push_back( Vec3f( randFloat( -8, 8 ), randFloat( 0, 8 ), randFloat( -8, 8 ) ) );
    
    BSpline3f spline( points, deg, false, false );
    SplinePath p = { spline, 0.0f, radius };
    
    mPaths.push_back( p );
}

