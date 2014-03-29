/*
 *  Fixture.h
 *
 *  Created by Andrea Cuius
 *	www.nocte.co.uk
 *
 */

#pragma once

#include "cinder/app/AppNative.h"
#include "cinder/gl/Vbo.h"
#include "cinder/ObjLoader.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>


class Fixture;
typedef std::shared_ptr<Fixture> FixtureRef;

class Fixture {
    
public: 
    
    static FixtureRef create( ci::Vec3f pos )
    {
        return FixtureRef( new Fixture( pos ) );
    }
    
    ~Fixture() {}
    
    virtual void update( float fadeInSpeed, float fadeOutSpeed )
    {
        if ( mValue < mTargetValue ) 
            mValue = std::min( mValue + fadeInSpeed, mTargetValue );
        else 
            mValue = std::max( mValue - fadeOutSpeed , mTargetValue );
    }
    
    ci::Vec3f   getPos() { return mPos; }
    ci::Vec2f	getPos2f() { return mPos.xz(); }
    
    float       getValue() { return mValue; }
    void        setValue( float val ) { mTargetValue = val; }
    void        addValue( float val ) { mTargetValue = std::min( std::max( mTargetValue + val, 0.0f ), 1.0f ); }
    
public:

	static std::vector<FixtureRef> loadFixtures( ci::fs::path filePath, bool flipZ = true )
	 {
		 ci::app::console() << "Load fixtures file: " << filePath << std::endl;
    
		std::vector<FixtureRef> fixtures;	

		std::ifstream openFile( filePath.generic_string().c_str() );						// open file stream
    
		ci::Vec3f   pos;
		std::string line;
		int         c = 0;
    
		if ( openFile.is_open() )                                                           // read file and parse comma separated values, one fixture per line
		{
			while ( openFile.good() )
			{
				getline(openFile,line);                                                     // get line
            
				std::vector<std::string> splitValues;                                       // split comma separated values
				boost::split(splitValues, line, boost::is_any_of(","));
            
				if ( splitValues.size() < 3 )                                               // we only import XYZ, check the line contains 3 values
				{
					ci::app::console() << "Error parsing line #" << c << std::endl;
					ci::app::console() << line << std::endl;
                
					c++;
					continue;
				}
            
				pos.x = boost::lexical_cast<float>(splitValues.at(0));                      // get X
				pos.y = boost::lexical_cast<float>(splitValues.at(1));                      // get Y
				pos.z = boost::lexical_cast<float>(splitValues.at(2));                      // get Z
            
				if ( flipZ ) pos.z *= -1;                                                   // sometimes we need to flip the Z coords
            
            
				fixtures.push_back( Fixture::create( pos ) );    // create a new Fixture
            
				c++;
			}
        
			openFile.close();
		}
		else
			ci::app::console() << "Failed loading file: " << filePath.generic_string() << std::endl;
    
		ci::app::console() << "Loaded " << fixtures.size() << " fixtures" << std::endl;
		
		return fixtures;
	 }
    
    static ci::gl::VboMeshRef loadObj( ci::fs::path filePath )
	{
		ci::gl::VboMeshRef  vboMesh;

		ci::ObjLoader loader( (ci::DataSourceRef)ci::loadFile( filePath ) );	            // load .obj file
    
		ci::TriMesh	mesh;																	// load TriMesh
		loader.load( &mesh );
    
		vboMesh = ci::gl::VboMesh::create( mesh );											// create VBO mesh shared_ptr
    
		ci::app::console() << "Loaded mesh: " << filePath.generic_string() << std::endl;
    
		return vboMesh;
	}
    
    static void renderGrid()
	{
		int     steps           = 10;                               // sizes in meters
		float   size            = 1.0f;
		float   halfLineLength  = size * steps * 0.5f;              // half line length
    
		ci::gl::color( ci::Color::gray( 0.4f ) );
    
		for( float i = -halfLineLength; i <= halfLineLength; i += size )
		{
			ci::gl::drawLine( ci::Vec3f( i, 0.0f, -halfLineLength ), ci::Vec3f( i, 0.0f, halfLineLength ) );
			ci::gl::drawLine( ci::Vec3f( -halfLineLength, 0.0f, i ), ci::Vec3f( halfLineLength, 0.0f, i ) );
		}
    
		ci::gl::drawCoordinateFrame();  
	}
    

protected:

    Fixture( ci::Vec3f pos ) : mPos(pos)
    {
        mValue          = 0.0f;
        mTargetValue    = 0.0f;
    }

    
protected:
    
    ci::Vec3f           mPos;                   // position in 3d space in meters
    float               mValue;                 // value normalised
    float               mTargetValue;           // target value for fading function

};
