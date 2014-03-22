/*
 *  SpotlightModule.cpp
 *
 *  Created by Andrea Cuius
 *  www.nocte.co.uk
 *
 */


// class forward declarations
class Fixture;
typedef std::shared_ptr<Fixture>    FixtureRef;

class SpotlightModule;
typedef std::shared_ptr<SpotlightModule>      SpotlightModuleRef;


class SpotlightModule {
	
public:
    
    static SpotlightModuleRef create( ci::Vec3f pos, float radius )
    {
        return SpotlightModuleRef( new SpotlightModule( pos, radius ) );
    }
    
    ~SpotlightModule() {}
    
    void update( std::vector<FixtureRef> fixtures, ci::Vec3f pos, float radius );
	
	void render();
    
    bool containsPoint( ci::Vec2f pos2f );

    
protected:
    
	SpotlightModule( ci::Vec3f pos, float radius );
    
    
private:

    ci::Vec3f   mPos;
    float       mRadius;
    
};


