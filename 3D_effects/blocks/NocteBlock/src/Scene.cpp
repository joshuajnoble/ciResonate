/*
 *  Scene.h
 *	NocteBlock
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Copyright 2012 . All rights reserved.
 *	www.nocte.co.uk
 *
 */

#include "cinder/App/App.h"
#include "cinder/ObjLoader.h"
#include "cinder/Utilities.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>

#include "Scene.h"

using namespace ci;
using namespace ci::app;
using namespace std;


Scene::Scene( fs::path fixturePath, fs::path venuePath )
{
    init( fixturePath, venuePath );
}


Scene::Scene( fs::path fixturesCoordsPath, bool flipZ, fs::path  fixturePath, fs::path venuePath )
{                   
    init( fixturePath, venuePath );
    
    importFixturesFile( fixturesCoordsPath, flipZ );
}
 
void Scene::init( fs::path fixturePath, fs::path venuePath )
{
    mFixtureMesh    = NULL;
    mVenueMesh      = NULL;
    mRenderGrid     = true;
    mMayaCam        = NULL;
    
    loadFixtureMesh( fixturePath );
    
    loadVenueMesh( venuePath );
    
    ci::app::WindowRef window = app::App::get()->getWindow();
    mCbMouseDown    = window->getSignalMouseDown().connect( std::bind( &Scene::mouseDown, this, std::placeholders::_1 ) );
    mCbMouseDrag    = window->getSignalMouseDrag().connect( std::bind( &Scene::mouseDrag, this, std::placeholders::_1 ) );
    mCbResize       = window->getSignalResize().connect( std::bind( &Scene::resize, this ) );
    
    ci::CameraPersp initialCam;
    initialCam.setPerspective( 45.0f, ci::app::getWindowAspectRatio(), 0.1, 3000 );
    mMayaCam        = new ci::MayaCamUI( initialCam );
    
    mGridSteps      = 10;
    mGridSize       = 1.0f;
    
    mGridColor      = ci::ColorA(0.2f, 0.2f, 0.2f, 1.0f);
    mFixtureColor   = ci::ColorA( 0.8f, 0.2f, 0.3f, 1.0f );
    mVenueColor     = ci::ColorA( 1.0f, 1.0f, 1.0f, 0.6f );
}


Scene::~Scene() 
{
    mFixtures.clear();
    
    delete mFixtureMesh;
    delete mVenueMesh;
    delete mMayaCam;
}

void Scene::loadFixtureMesh( fs::path filePath )
{
    mFixtureObjPath = filePath;
    
    if ( mFixtureMesh )
        delete mFixtureMesh;
    
    mFixtureMesh = NULL;
    
    ObjLoader loader( (DataSourceRef)loadFile( filePath ) );
    TriMesh	mesh; 
    loader.load( &mesh );
    mFixtureMesh = new gl::VboMesh( mesh ); 
    
    for( size_t i=0; i < mFixtures.size(); i++ )
        mFixtures[i]->setMesh(mFixtureMesh); 
}


void Scene::loadVenueMesh( fs::path filePath )
{
    mVenueObjPath = filePath;
    
    if ( mVenueMesh )
        delete mVenueMesh;
    
    mVenueMesh = NULL;
    
    ObjLoader loader( (DataSourceRef)loadFile( filePath ) );
    TriMesh	mesh; 
    loader.load( &mesh );
    mVenueMesh = new gl::VboMesh( mesh );   
}


void Scene::importFixturesFile( fs::path filePath, bool flipZ )
{
    mFixtureCoordsPath = filePath;
    
    string fname = filePath.generic_string();
    ifstream openFile( fname.c_str() );
    
    int         dmxChannel  = 0;
    int         c           = 0;
    
    ci::Vec3f   pos;
    std::string line;
    
    if (openFile.is_open())
    {                
        while ( openFile.good() )
        {
            getline(openFile,line);
            std::vector<std::string> splitValues;
            boost::split(splitValues, line, boost::is_any_of(","));						// get comma separated values
            
            if ( splitValues.size() < 3 )
            {
                console() << "Scene > Failed to parse fixtures file at line #" << c << endl;
                
                c++;

                continue;
            }
            
            pos.x = boost::lexical_cast<float>(splitValues.at(0));
            pos.y = boost::lexical_cast<float>(splitValues.at(1));
            pos.z = boost::lexical_cast<float>(splitValues.at(2));
            
            if ( flipZ ) pos.z *= -1;
            
//            pos /= scale;

            dmxChannel = ( splitValues.size() == 4 ) ? boost::lexical_cast<int>(splitValues.at(3)) : 0;
            
            mFixtures.push_back( Fixture::create( pos, dmxChannel, mFixtureMesh ) );
            
            c++;
        }
        
        openFile.close();
    } 
    else
        console() << "Scene > Failed to load fixtures coordinates: " << filePath.generic_string() << endl;
    
}
    

void Scene::begin( bool drawWireframe )
{
    gl::pushMatrices();
    
    gl::setMatrices( mMayaCam->getCamera() );
    
    if ( mRenderGrid )
    {
        gl::color( mGridColor );
        renderGrid( mGridSteps, mGridSize );
    }
    
    // render venue
    if ( mVenueMesh )
    {
        gl::color( mVenueColor );
        gl::draw( *mVenueMesh );
        
        if ( drawWireframe )
        {
            gl::enableWireframe();
            gl::draw( *mVenueMesh );
            gl::disableWireframe();
        }
    }
    
    // render fixtures
    ci::gl::color( mFixtureColor );
    for( size_t i=0; i < mFixtures.size(); i++ )
        mFixtures[i]->render();
    
    ci::gl::color( ci::Color::white() );
}


void Scene::end()
{
    gl::popMatrices();
}


void Scene::renderLabels()
{
    Vec2f           screenPos;
    CameraPersp     cam         = mMayaCam->getCamera();
    Vec2f           windowSize  = getWindowSize();
    
    gl::setMatricesWindow( windowSize );
    
    for( size_t k=0; k < mFixtures.size(); k++ )
    {
        screenPos = cam.worldToScreen ( mFixtures[k]->getPos(), windowSize.x, windowSize.y );
        gl::drawString( toString( mFixtures[k]->getChannel() ), screenPos );
    }   
}


void Scene::renderValues()
{
    Vec2f           screenPos;
    CameraPersp     cam         = mMayaCam->getCamera();
    Vec2f           windowSize  = getWindowSize();
    
    gl::setMatricesWindow( windowSize );
    
    for( size_t k=0; k < mFixtures.size(); k++ )
    {
        screenPos = cam.worldToScreen ( mFixtures[k]->getPos(), windowSize.x, windowSize.y );
        gl::drawString( toString( mFixtures[k]->getValue() ), screenPos );
    }  
}

