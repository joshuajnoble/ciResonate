/*
 *  Fixture.h
 *	NocteBlock
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Copyright 2012 . All rights reserved.
 *	www.nocte.co.uk
 *
 */

#ifndef NOCTE_FIXTURE
#define NOCTE_FIXTURE

#pragma once

#include "cinder/gl/Vbo.h"
#include "cinder/ObjLoader.h"

#define MIN_LAMP_VALUE      0.0035  // min value norm -> 1.0f / 255 = 0.00392
#define FIXTURE_OFF_COLOR   ci::ColorA( 1.0f, 1.0f, 1.0f, 0.1f )

class Fixture;
typedef std::shared_ptr<Fixture> FixtureRef;

class Fixture {
    
    friend class Scene;
    friend class Stage;
    
    
public: 
    
    static FixtureRef create( ci::Vec3f pos, int dmxChannel, ci::gl::VboMesh *mesh = NULL )
    {
        return FixtureRef( new Fixture( pos, dmxChannel, mesh ) );
    }
    
    ~Fixture() {}
    
    virtual void update( float val, float fadeInSpeed, float fadeOutSpeed )
    {                
        mTargetValue = ci::math<float>::clamp(val, 0.0f, 1.0f);
        
        if ( mValue < mTargetValue ) 
            mValue = std::min(mValue + fadeInSpeed, mTargetValue);
        else 
            mValue = std::max(mValue - fadeOutSpeed , mTargetValue);
        
        //        if ( mValue < MIN_LAMP_VALUE )
        //            mValue = 0.0f;
    }
    
    virtual void render() 
    {
        if ( !mMesh )
            return;
        
        if ( mValue > 0.0f )
            ci::gl::color( ci::ColorA( 0.9f, 0.21f, 0.34f, mValue ) );
        else
            ci::gl::color( FIXTURE_OFF_COLOR );
        
        ci::gl::pushMatrices();
        ci::gl::translate(mPos);
        ci::gl::draw( *mMesh );
        ci::gl::popMatrices();
    }
    
    ci::Vec3f   getPos() { return mPos; }
    
    ci::Vec2f	getPos2f() { return mPos.xz(); }
    
    float getValue() { return mValue; }
    
    float getTargetValue() { return mTargetValue; }
    
    int getChannel() { return mDMXChannel; }
      
    void setMesh( ci::gl::VboMesh *mesh ) { mMesh = mesh; }
    
    void loadMesh( const std::string &meshRes )
    {
        ci::ObjLoader loader( (ci::DataSourceRef)ci::app::loadResource(meshRes) );
        ci::TriMesh	mesh; 
        loader.load( &mesh );
        mMesh = new ci::gl::VboMesh( mesh );
    }
    

protected:

    Fixture( ci::Vec3f pos, int dmxChannel, ci::gl::VboMesh *mesh = NULL )
    : mPos(pos), mDMXChannel(dmxChannel), mValue(0.0f), mTargetValue(0.0f), mMesh(mesh) {}

    
protected:
    
    ci::Vec3f           mPos;
    int                 mDMXChannel;
    float               mValue;             // val normalised
    float               mTargetValue;
    ci::gl::VboMesh     *mMesh;
    
private:
    // disallow
    Fixture(const Fixture&);
    Fixture& operator=(const Fixture&);

};


#endif