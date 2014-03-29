#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "cinder/ImageIo.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class LightsApp : public AppNative {
  public:
    
    void prepareSettings(Settings *settings);
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
    gl::GlslProg mShader;
    gl::Texture mTex;
    params::InterfaceGl mParams, mParams2;
    
    Vec3f mLight1Position, mLight2Position;
    ColorAf mLight1Color, mLight2Color;
    float mLight1Type, mLight2Type;
    Vec3f mLight1Direction, mLight2Direction;

    
};

void LightsApp::prepareSettings(Settings *settings)
{
    settings->setWindowSize(800, 600);
}

void LightsApp::setup()
{
    
    
    try
    {
        mShader = gl::GlslProg( loadAsset("lighting.vert"), loadAsset("lighting.frag") );
    }
    catch ( gl::GlslProgCompileExc err )
    {
        cout << err.what() << endl;
    }
    mTex = gl::Texture( loadImage( loadAsset("mona_lisa.jpg")));
    
    mLight1Position = Vec3f(0,100,100);
    mLight2Position = Vec3f(800,100,100);
    mLight1Color = Colorf(1.f, 0.f, 0.f);
    mLight2Color = Colorf(0.f, 0.f, 1.f);
    
    mLight1Type = 2;
    mLight2Type = 2;
    
    mLight1Direction = Vec3f(0,1,0);
    mLight2Direction = Vec3f(0,0,1);
    
    mParams = params::InterfaceGl( getWindow(), "App parameters", toPixels( Vec2i( 200, 200 ) ) );
	mParams.addParam( "Light 1 Position ", &mLight1Position, "");
    mParams.addParam( "Light 1 Type ", &mLight1Type, "");
    mParams.addParam( "Light 1 Direction ", &mLight1Direction, "");
    mParams.addParam( "Light 1 Color ", &mLight1Color, "");
    
    //mParams2 = params::InterfaceGl( getWindow(), "App parameters", toPixels( Vec2i( 200, 200 ) ) );
    
	mParams.addParam( "Light 2 Position ", &mLight2Position, "");
    mParams.addParam( "Light 2 Type ", &mLight2Type, "");
    mParams.addParam( "Light 2 Direction ", &mLight2Direction, "");
    mParams.addParam( "Light 2 Color ", &mLight2Color, "");
    
}

void LightsApp::mouseDown( MouseEvent event )
{
}

void LightsApp::update()
{
}

void LightsApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    mShader.bind();
    
    gl::pushMatrices();
    
    mShader.uniform("lights[0].type", mLight1Type);
    mShader.uniform("lights[0].spotDirection", mLight1Direction);
    Vec4f lp1(mLight1Position.x, mLight1Position.y, mLight1Position.z, 1.0);
    mShader.uniform("lights[0].position", lp1);

    // tying these all together for the moment
    Vec4f ambient(0, 0., 0, 1.0);
    mShader.uniform("lights[0].ambient", mLight1Color);
    Vec4f specular(1, 1, 0, 1.0);
    mShader.uniform("lights[0].specular", mLight1Color);
    Vec4f diffuse(0, 1, 0, 1.0);
    mShader.uniform("lights[0].diffuse", mLight1Color);
    
    //float constantAttenuation = 0.000001;
    //float linearAttenuation = 0.000001;
    //float quadraticAttenuation = 0.000001;

	float constantAttenuation = 0.0001;
    float linearAttenuation = 0.0001;
    float quadraticAttenuation = 0.0001;
    
    mShader.uniform("lights[0].constantAttenuation", constantAttenuation);
    mShader.uniform("lights[0].linearAttenuation", linearAttenuation);
    mShader.uniform("lights[0].quadraticAttenuation", quadraticAttenuation);
    
    //// 2 light
    
    mShader.uniform("lights[1].type", mLight2Type);
    mShader.uniform("lights[1].spotDirection", mLight2Direction);
    
    
    Vec4f lp2(mLight2Position.x, mLight2Position.y, mLight2Position.z, 1.0);
    mShader.uniform("lights[1].position", lp2);
    
    // tying these all together for the moment
    Vec4f ambient2(1, 0, 1, 1.0);
    mShader.uniform("lights[1].ambient", mLight2Color);
    Vec4f specular2(1, 0, 1, 1.0);
    mShader.uniform("lights[1].specular", mLight2Color);
    Vec4f diffuse2(1, 0, 1, 1.0);
    mShader.uniform("lights[1].diffuse", mLight2Color);
    
    mShader.uniform("lights[1].constantAttenuation", constantAttenuation);
    mShader.uniform("lights[1].linearAttenuation", linearAttenuation);
    mShader.uniform("lights[1].quadraticAttenuation", quadraticAttenuation);
    
    
    //currentMaterial
    Vec4f matAmbient(1.f, 1.f, 1.f, 1.f);
    mShader.uniform("mat_ambient", matAmbient);
    Vec4f matSpecular(.8f, .8f, .8f, 1.f);
    mShader.uniform("mat_diffuse", matSpecular);
    Vec4f matDiffuse(1.0f, 1.0f, 1.f, 1.0f);
    mShader.uniform("mat_specular", matDiffuse);
    //ofVec4f matShininess(.5f, 0.5f, 0.5f, .5f);
    float matShininess = 0.5;
    mShader.uniform("mat_shininess", matShininess);
    
    gl::drawSphere( Vec3f(300, 200, 0), 100, 64);
    
    mShader.unbind();
    
    gl::popMatrices();
    
    mParams.draw();
}

CINDER_APP_NATIVE( LightsApp, RendererGl )
