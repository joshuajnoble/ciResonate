/*
 *  Scene.h
 *	NocteBlock
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Copyright 2012 . All rights reserved.
 *	www.nocte.co.uk
 *
 */

#ifndef NOCTE_SCENE
#define NOCTE_SCENE

#pragma once

#include "cinder/MayaCamUI.h"
#include "cinder/gl/Vbo.h"
#include "Fixture.h"

class Scene;
typedef std::shared_ptr<Scene>  SceneRef;


class Scene {

    friend class Stage;
    
public:
    
    static SceneRef create( ci::fs::path fixturePath, ci::fs::path venuePath )
    {
        return SceneRef( new Scene( fixturePath, venuePath ) );
    }
    
    static SceneRef create( ci::fs::path fixturesCoordsPath, bool flipZ, ci::fs::path fixturePath, ci::fs::path venuePath )
    {
        return SceneRef( new Scene( fixturesCoordsPath, flipZ, fixturePath, venuePath ) );
    }
    
    ~Scene();
    
    void init( ci::fs::path fixturePath, ci::fs::path venuePath );
    
    void render();
    
    void update( std::shared_ptr<float> values, float fadeIn = 1.0f, float fadeOut = 1.0f )
    {	
        for(int k=0; k < mFixtures.size(); k++) 
            mFixtures[k]->update( values.get()[k], fadeIn, fadeOut );
    }
    
    int getFixturesN() { return mFixtures.size(); }
    
    FixtureRef getFixture( int n ) { return mFixtures[n]; }
    
    std::vector<FixtureRef> getFixtures() { return mFixtures; }
    
    void begin( bool drawWireframe = false );
    
    void end();
    
    void renderLabels();
    
    void renderValues();
    
    ci::MayaCamUI*  getMayaCam() { return mMayaCam; }
    
    ci::CameraPersp getCamera() { return mMayaCam->getCamera(); }
    
    void setCamera( ci::CameraPersp cam ) { mMayaCam->setCurrentCam( cam ); }
    
    void renderFrustum( ci::ColorA col = ci::ColorA::white() )
    {
        ci::gl::color( col );
        ci::gl::drawFrustum( mMayaCam->getCamera() );
        ci::gl::color( ci::Color::white() );
    }   
    
    void toggleGrid() { mRenderGrid = !mRenderGrid; }

    void setGridSize( int steps=10, float size=1.0f ) { mGridSteps = steps; mGridSize = size; }
    
    void setGridColor( ci::ColorA col ) { mGridColor = col; }
    void setFixtureColor( ci::ColorA col ) { mFixtureColor = col; }
    void setVenueColor( ci::ColorA col ) { mVenueColor = col; }
    
    void addFixture( FixtureRef fixture, bool setMesh = true )         // temp method, this is to load subclass of Fixture
    { 
        if ( setMesh )
            fixture->setMesh(mFixtureMesh);

        mFixtures.push_back( fixture );
    } 
    
    void setFixtureValue( int idx, float val )
    {
        if ( idx < mFixtures.size() )
            mFixtures[idx]->mValue = ci::math<float>::clamp( val, 0.0f, 1.0f );
    }
    
    ci::fs::path getVenueObjPath() { return mVenueObjPath; }
    
    ci::fs::path getFixtureObjPath() { return mFixtureObjPath; }
    
    ci::fs::path getFixtureCoordsPath() { return mFixtureCoordsPath; }
    
    
protected:
    
    Scene( ci::fs::path fixturePath, ci::fs::path venuePath );
    
    Scene( ci::fs::path fixturesCoordsPath, bool flipZ, ci::fs::path fixturePath, ci::fs::path venuePath );
    
    void importFixturesFile( ci::fs::path filePath, bool flipZ );
    
    void loadFixtureMesh( ci::fs::path filePath );
    
    void loadVenueMesh( ci::fs::path filePath );
    
    void renderGrid( int steps=10, float size=1.0f )      // size in meters
    {
        ci::gl::color( mGridColor );
        
        float halfLineLength = size * steps * 0.5f;     // half line length
        
        for(float i=-halfLineLength; i<=halfLineLength; i+=size) {
            ci::gl::drawLine( ci::Vec3f(i, 0.0f, -halfLineLength), ci::Vec3f(i, 0.0f, halfLineLength) );
            ci::gl::drawLine( ci::Vec3f(-halfLineLength, 0.0f, i), ci::Vec3f(halfLineLength, 0.0f, i) );
        }
        
        ci::gl::drawCoordinateFrame();
    }
    
    bool mouseDown( ci::app::MouseEvent event )
    {
        if( event.isAltDown() )
        {
            mMayaCam->mouseDown( event.getPos() );
            return true;
        }
        
        return false;
    }
    
    bool mouseDrag( ci::app::MouseEvent event )
    {
        if( event.isAltDown() )
        {
            mMayaCam->mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
            return true;
        }
        return false;            
    }
    
    bool resize()
    {
        ci::CameraPersp cam = mMayaCam->getCamera();
        cam.setPerspective( 45.0f, ci::app::getWindowAspectRatio(), 0.1, 3000 );
        mMayaCam->setCurrentCam( cam );
        return true;
    }
    
    
private:
    
    std::vector<FixtureRef>     mFixtures;
    ci::gl::VboMesh             *mFixtureMesh;
    ci::gl::VboMesh             *mVenueMesh;
    
    ci::signals::scoped_connection	mCbMouseDown, mCbMouseDrag, mCbResize;
    
    ci::MayaCamUI               *mMayaCam;
    bool                        mRenderGrid;
    int                         mGridSteps;
    float                       mGridSize;
    ci::ColorA                  mGridColor;
    ci::ColorA                  mFixtureColor;
    ci::ColorA                  mVenueColor;
    ci::fs::path                mVenueObjPath;
    ci::fs::path                mFixtureObjPath;
    ci::fs::path                mFixtureCoordsPath;
    
};


#endif