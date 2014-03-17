/*
 *  Fixture.h
 *
 *  Created by Andrea Cuius
 *	www.nocte.co.uk
 *
 */

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
