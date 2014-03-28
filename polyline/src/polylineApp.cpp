
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/CinderMath.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// Make it so that the longer you hold in one spot the further back in Z you go
// and then rotate allow as well

float sizeConstant = 400.f;
bool mousePressed = false;

class SketchLine  {
    
public:
    
    float mElastConst, mDecayConst;
    
    int mLength;
    
    vector<PolyLine<Vec3f>> mLocation;
    vector<PolyLine<Vec3f>>::iterator mLastLine;
    
    vector<float> mElastic;
    vector<float>::iterator mElasticIt;
    
    vector<float> mDecay;
    vector<float>::iterator mDecayIt;
    
    Vec3f mStart;
    list<Vec3f> mPointsToAdd;
    
    void setup( int len );
    
    void addPoint( const Vec2f & s, float timeAtCurrentPosition);
    void calcType(float mElasticVar, float decayVar);
    void calcPoints();
    void calcPointsStart(const Vec2f & mouse, float depth);
    void calcRemainingPoints(const Vec2f & mouse, float timeAtLastPoint);
    void render(Color & color);
    
};
void SketchLine::setup( int len )
{
    mLength = len;
    
//    PolyLine<Vec3f> firstLine;
//    mLocation.push_back(firstLine);
//    mLastLine = mLocation.end() - 1;

    mElastic.insert(mElastic.begin(), mLength, 0.f);
    mDecay.insert(mDecay.begin(), mLength, 0);
    
}

void SketchLine::calcType(float mElasticVar, float decayVar)  {
    mElastConst = mElasticVar;
    mDecayConst = decayVar;
    
    for (int i=0; i < mLength; i++)
    {
        mElastic[i] = mElastConst*(.07*(i+1));// 0.05  kai 0.005
        cout << mElastic[i] << endl;
        mDecay[i] = mDecayConst-(0.02*i);
    }
    
    mElasticIt = mElastic.begin();
    mDecayIt = mDecay.begin();
}

void SketchLine::addPoint( const Vec2f & s, float timeAtCurrentPosition )
{
    Vec3f v(s + (s * (*mElasticIt) * (*mDecayIt)));
    //v.z = constrain<float>(timeAtCurrentPosition * 100.f, -400, 400);
    v.z = timeAtCurrentPosition * sizeConstant;
    
    cout << v << endl;
    
    mLastLine->push_back( v );
    
    ++mElasticIt;
    if(mElasticIt == mElastic.end()) {
        mElasticIt = mElastic.begin();
    }
    
    ++mDecayIt;
    if(mDecayIt == mDecay.end()) {
        mDecayIt = mDecay.begin();
    }
}

void SketchLine::calcPoints(){
    
    if(mPointsToAdd.size() == 0) {
        return;
    }
    
    Vec3f v = mPointsToAdd.front();
    mPointsToAdd.erase(mPointsToAdd.begin());
    mLastLine->push_back(v);
}

void SketchLine::calcPointsStart(const Vec2f & mouse, float depth)  {
    mStart.set(mouse.x, mouse.y, depth * sizeConstant);
    
    PolyLine<Vec3f> newLine;
    mLocation.push_back(newLine);
    mLastLine = mLocation.end() - 1;
    
}


// figure out all the points whenever there's not a mouse movement
// then add them in one at a time until the mouse is dragged again,
// at which point make 30 new ones and add them all in
void SketchLine::calcRemainingPoints(const Vec2f & mouse, float timeAtLastPoint)
{
    
    Vec3f v(mouse);
    v.z = timeAtLastPoint * sizeConstant;
    mLastLine->push_back(v);
    
    while(mPointsToAdd.size() < mLength)
    {
        mPointsToAdd.push_back(Vec3f(0,0,0));
    }
    
    vector<Vec3f>::iterator rIt = mLastLine->getPoints().end();
    rIt -= min( (int) mLastLine->getPoints().size(), mLength);
    
    if(rIt == mLastLine->getPoints().end() || rIt+1 == mLastLine->getPoints().end()) {
        return;
    }
    
    Vec3f lastPoint = v;

    float decayAmount = 1.f/(float) mPointsToAdd.size();
    float linearDecay = -1.f;
    //float distanceFromInit = 0.1;
    
    for ( list<Vec3f>::iterator pIt = mPointsToAdd.begin(); pIt != mPointsToAdd.end(); ++pIt)
    {
        
        Vec3f c = lastPoint;
        Vec3f delta;



        delta = ( *(rIt) - *(rIt+1));
        c += (delta * linearDecay);
        
        pIt->set(c);
        
        if(rIt != mLastLine->getPoints().end() && rIt+1 == mLastLine->getPoints().end()) {
            ++rIt;
        }

        
        lastPoint = c;
        linearDecay += decayAmount;
    }
}

void SketchLine::render(Color & color)
{
    gl::color(color);
    
    for( int i = 0; i < mLocation.size(); i++ ) {
        gl::draw(mLocation[i]);
    }
    
}


class polylineApp : public AppNative {
public:
    
	void setup();
	void mouseDown( MouseEvent event );
    void mouseDrag(cinder::app::MouseEvent event);
    void mouseUp( MouseEvent event );
	void update();
	void draw();
    
    bool mousePressed;
    
    SketchLine mLines[3];
    
    Vec2f mouse;
    Vec2f points[50];
    Vec2f accel[50];
    Vec2f delta[50];
    
    Color one, two, three;
    
    float lastMouseDrag;
    float mRotation;
    
};

void polylineApp::setup()
{

	mRotation = 0;
    
    mousePressed = false;
    
    mLines[0].setup(30);
    mLines[1].setup(31);
    mLines[2].setup(32);
    
    mLines[0].calcType( 0.2, 0.65 );
    mLines[1].calcType( 0.1, 0.65 );
    mLines[2].calcType( 0.3, 0.65 );
    
    one.set(CM_HSV, Vec3f(126.0/360.0, 1.0, 1.0));
    three.set(CM_HSV, Vec3f(57.0/360.0, 1.0, 1.0 ));
    two.set(CM_HSV, Vec3f(303.0/360.0, 1.0, 1.0 ));
    gl::clear();
    
    mouse.x = 300;
    
}

void polylineApp::mouseUp( MouseEvent event )
{
    mousePressed = false;
    
    float depth = tan( mRotation * 0.01745);

    mLines[0].calcRemainingPoints(mouse, depth);
    mLines[1].calcRemainingPoints(mouse, depth);
    mLines[2].calcRemainingPoints(mouse, depth);
}

void polylineApp::mouseDown( MouseEvent event )
{
    mousePressed = true;
    mouse = event.getPos();
    
    float depth = tan( mRotation * 0.01745);
    
    mLines[0].calcPointsStart(mouse,depth);
    mLines[1].calcPointsStart(mouse,depth);
    mLines[2].calcPointsStart(mouse,depth);
}

void polylineApp::mouseDrag(cinder::app::MouseEvent event)
{
    mouse = event.getPos();
    
    //float depth = tan( mRotation * 0.01745);
    //
    //mLines[0].addPoint(mouse, depth);
    //mLines[1].addPoint(mouse, depth);
    //mLines[2].addPoint(mouse, depth);

    //lastMouseDrag = getElapsedSeconds();
    //
}

void polylineApp::update()
{
    
    if(!mousePressed)
    {
        mLines[0].calcPoints();
        mLines[1].calcPoints();
        mLines[2].calcPoints();
    }
    
    if(mousePressed)
    {
        float depth = tan( mRotation * 0.01745 );

        mLines[0].addPoint(mouse, depth);
        mLines[1].addPoint(mouse, depth);
        mLines[2].addPoint(mouse, depth);
    }
    
    mRotation += 0.2;
    
}

void polylineApp::draw()
{
    
    
    gl::clear();
    
    gl::pushMatrices();
    gl::translate( getWindowWidth()/2, 0, -800 );
    Vec3f rotationVector(0, mRotation, 0);
    gl::rotate(rotationVector);
    
    mLines[0].render(one);
    mLines[1].render(two);
    mLines[2].render(three);
    
    gl::popMatrices();
    
}
CINDER_APP_NATIVE( polylineApp, RendererGl )
