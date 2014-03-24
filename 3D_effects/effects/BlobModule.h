/*
 *  BlobModule.cpp
 *
 *  Created by Andrea Cuius
 *  www.nocte.co.uk
 *
 */

#include "Fixture.h"

class BlobModule;
typedef std::shared_ptr<BlobModule>      BlobModuleRef;


class BlobModule {
	
public:
    
    static BlobModuleRef create( ci::Vec3f pos, int pointsN )
    {
        return BlobModuleRef( new BlobModule( pos, pointsN ) );
    }
    
    ~BlobModule() {}
    
    void update( std::vector<FixtureRef> fixtures, float speed, float radius );
	
	void render();
    
    bool containsPoint( ci::Vec2f pos2f );

    
protected:
    
	BlobModule( ci::Vec3f pos, int pointsN );
    
    
private:

    struct Point2f {
        ci::Vec2f	pos;
        ci::Vec2f	dir;
        ci::Vec2f	vel;
    };

	
    ci::Vec3f               mPos;
    std::vector<Point2f>	mPoints;
    
};


