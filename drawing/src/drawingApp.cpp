#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

bool mousePressed = false;




class SketchLine  {
    
public:
    
    float mElastConst, mDecayConst;
    
    int mLength;
    
    vector<Vec2f> mLocation;
    vector<Vec2f> mCalculated;
    vector<Vec2f> mDelta;
    
    vector<float> mElastic;
    vector<float> mDecay;
    
    Vec2f start;
    
    void setup( int len );
    
    void calcType(float elastic, float decay );
    void calcPoints( const Vec2f & s );
    void calcPointsStart(const Vec2f & mouse);
    void render(Color & color);
    
};

void SketchLine::setup( int len )
{
    mLength = len;
    
    mLocation.insert(mLocation.begin(), mLength, Vec2f(0,0));
    mCalculated.insert(mCalculated.begin(), mLength, Vec2f(0,0));
    mDelta.insert(mDelta.begin(), mLength, Vec2f(0,0));
    
    mElastic.insert(mElastic.begin(), mLength, 0.f);
    mDecay.insert(mDecay.begin(), mLength, 0);
    
}

void SketchLine::calcType(float elastic, float decay)  {
    mElastConst = elastic;
    mDecayConst = decay;
    for (int i=0; i < mLength; i++)
    {
        mElastic[i] = mElastConst*(.07*(i+1));// 0.05  kai 0.005
        mDecay[i] = mDecayConst-(0.02*i);
    }
}

void SketchLine::calcPoints( const Vec2f & s )  {
    start = s;
    
    for (int i=0; i < mLength; i++){
        if (i==0){
            mDelta[i] = (start - mLocation[i]);
        }
        else  {
            mDelta[i] = (mLocation[i-1] - mLocation[i]);
        }
        mDelta[i] *= mElastic[i];    // create mElastic effect
        mCalculated[i] += mDelta[i];
        mLocation[i] += mCalculated[i];// move it
        mCalculated[i] *= mDecay[i];    // slow down mElastic
    }
}
void SketchLine::calcPointsStart(const Vec2f & mouse)  {
    start = mouse;
    for (int i=0; i<mLength; i++){
        mLocation[i] = mouse;
    }
}


void SketchLine::render(Color & color)
{
    gl::color(color);

    for (int i = 0; i < mLength; i++)  {
        
        //int size = i == 0 ? 4 : (mLocation[i-1].distance(mLocation[i]) / 2.0);
        //gl::drawSolidCircle(mLocation[i], size);
        
        //gl::drawSolidEllipse( Vec2f(1.0,1.0), 1.0, 1.0);
        gl::drawSolidEllipse( mLocation[i], fabs(mLocation[i-1].x - mLocation[i].x) / 10.0, fabs( mLocation[i-1].y - mLocation[i].y ) / 10.0 );
    }
    
}


class drawingApp : public AppNative {
  public:
    
    drawingApp();
	void setup();
	void mouseDown( MouseEvent event );
    void mouseDrag(cinder::app::MouseEvent event);
    void mouseUp( MouseEvent event );
	void update();
	void draw();
    
    
    SketchLine mLine[3];
    
    float mElasticness[50];
    float mDecay[50];
    
    Vec2f mMouse;
    
    Color one, two, three;
    
};

drawingApp::drawingApp()
{}

void drawingApp::setup()
{
    mLine[0].setup(30);
    mLine[1].setup(31);
    mLine[2].setup(32);
    
    for (int i=0; i<50; i++){
        mElasticness[i] = 0.2*(.07*(i+1));// 0.05  kai 0.005
        mDecay[i] = 0.55-(0.02*i);
    }
    
    mLine[0].calcType( 0.2, 0.65 );
    mLine[1].calcType( 0.1, 0.65 );
    mLine[2].calcType( 0.3, 0.65 );
    
    one.set(CM_HSV, Vec3f(126.0/360.0, 1.0, 1.0));
    three.set(CM_HSV, Vec3f(57.0/360.0, 1.0, 1.0 ));
    two.set(CM_HSV, Vec3f(303.0/360.0, 1.0, 1.0 ));
    gl::clear();

}

void drawingApp::mouseUp( MouseEvent event )
{
}

void drawingApp::mouseDown( MouseEvent event )
{
    
    mMouse = event.getPos();
    
    mLine[0].calcPointsStart(mMouse);
    mLine[1].calcPointsStart(mMouse);
    mLine[2].calcPointsStart(mMouse);
}

void drawingApp::mouseDrag(cinder::app::MouseEvent event)
{
    mMouse = event.getPos();

}

void drawingApp::update()
{
    mLine[0].calcPoints(mMouse);
    mLine[1].calcPoints(mMouse);
    mLine[2].calcPoints(mMouse);
}

void drawingApp::draw()
{
    
    mLine[0].render(one);
    mLine[1].render(two);
    mLine[2].render(three);
    
}

CINDER_APP_NATIVE( drawingApp, RendererGl )