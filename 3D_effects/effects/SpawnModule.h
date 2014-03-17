/*
 *  SpawnModule.h
 *
 *  Created by Andrea Cuius
 *  www.nocte.co.uk
 *
 */


// class forward declarations
class Fixture;
typedef std::shared_ptr<Fixture>    	FixtureRef;

class SpawnModule;
typedef std::shared_ptr<SpawnModule>	SpawnModuleRef;


class SpawnModule {
	
public:
    
    static SpawnModuleRef create()
    {
        return SpawnModuleRef( new SpawnModule() );
    }
    
    ~SpawnModule() {}
    
    void update( std::vector<FixtureRef> fixtures, float speed );
	
	void render();
    
    bool containsPoint( ci::Vec2f pos2f );

    void createSpline();
    
    void createPath( int n, float radius );
    
protected:
    
	SpawnModule();
    
private:
    
    struct SplinePath {
        ci::BSpline3f   spline;
        float           posNorm;
        float           radius;
        
        ci::Vec3f       getCurrentPos() { return spline.getPosition( posNorm ); }
    };
    
    std::vector<SplinePath>     mPaths;
    
};