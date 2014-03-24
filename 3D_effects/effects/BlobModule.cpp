/*
 *  BlobModule.cpp
 *
 *  Created by Andrea Cuius
 *  www.nocte.co.uk
 *
 */

#include "BlobModule.h"

using namespace ci;
using namespace ci::app;
using namespace std;


BlobModule::BlobModule( Vec3f pos, int pointsN )
{    
    mPos    = pos;
    
    for ( int k=0; k < pointsN; k++ )                           // create n points
    {
        Point2f point;
        
        float angle = 2.0f * M_PI * (float)k / (float)pointsN;
        point.pos   = ci::Vec2f( cos(angle), sin(angle) );      // pos and dir are normalised
        point.dir   = point.pos;
        
        mPoints.push_back( point );
    }
}


void BlobModule::update( vector<FixtureRef> fixtures, float speed, float radius )
{
    // update points
    float length, vel;
    for ( size_t k=0; k < mPoints.size(); k++ )
    {
        vel          =  speed * 0.5f * ( 1.0f + cos( mPoints[k].dir.y + mPoints[k].dir.x + getElapsedSeconds() ) );
//        vel             = speed * ( mPoints[k].dir.y + 2.0f * mPoints[k].dir.x + getElapsedSeconds() );
        length          = radius * cos( vel );
        
        mPoints[k].pos  = length * mPoints[k].dir;
    }
    
    // update Fixture values
    float val;
    for ( size_t k=0; k < fixtures.size(); k++ )
    {
        if ( containsPoint( fixtures[k]->getPos2f() ) )
            val =  1.0f;
        else
            val = 0.0f;
        
        fixtures[k]->setValue( val );
    }
}


void BlobModule::render()
{
    gl::color( Color( 1.0f, 0.0f, 0.0f ) );
    gl::drawCube( mPos, Vec3f::one() * 0.2f );
    
    gl::color( Color( 0.0f, 1.0f, 1.0f ) );
    glBegin(GL_LINE_LOOP);
    for( size_t k=0; k < mPoints.size(); k++ )
        gl::vertex( mPos + Vec3f( mPoints[k].pos.x, 0, mPoints[k].pos.y ) );
    glEnd();
    
	gl::color( Color::white() );
}


bool BlobModule::containsPoint( Vec2f pos2f )
{
    int i, j, c = 0;
    Vec2f posJ, posI;
    
    for ( i = 0, j = mPoints.size()-1; i < mPoints.size(); j = i++ )
    {
        posJ = mPos.xz() + mPoints[j].pos;
        posI = mPos.xz() + mPoints[i].pos;
        
        if ( ( (posI.y > pos2f.y) != (posJ.y > pos2f.y)) &&
            (pos2f.x < (posJ.x - posI.x) * (pos2f.y - posI.y) / (posJ.y-posI.y) + posI.x) )
            c = !c;
    }
    
    if (c)
        return true;
    
    
    return c;
}
