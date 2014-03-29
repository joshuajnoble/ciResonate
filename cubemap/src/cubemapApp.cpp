#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"
#include "MeshHelper.h"
#include "CubeMap.h"
#include "cinder/params/Params.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class cubemapApp : public AppNative {
  public:
    
    void prepareSettings( Settings * settings);
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
    gl::VboMesh mVbo;
    MeshHelper mMeshHelper;
    CubeMap *mCubeMap;
    gl::GlslProg mShader;
    
    float mDiffuseK, mRoughness, mIndexOfRefraction, mSpecularAttenuation, mTransluscence;
	Colorf mColorAmbient, mColorDiffuse, mColorSpecular;
	Vec3f mLightPosition;
    
    params::InterfaceGl mControls;
};

void cubemapApp::prepareSettings( Settings * settings)
{

    settings->setWindowSize(1000,800);

}

void cubemapApp::setup()
{
    mCubeMap = new CubeMap( GLsizei(512), GLsizei(512),
                       Surface8u( loadImage( loadResource( "grandcanyon_positive_x.jpg" ) ) ),
                       Surface8u( loadImage( loadResource( "grandcanyon_positive_y.jpg" ) ) ),
                       Surface8u( loadImage( loadResource( "grandcanyon_positive_z.jpg" ) ) ),
                       Surface8u( loadImage( loadResource( "grandcanyon_negative_x.jpg" ) ) ),
                       Surface8u( loadImage( loadResource( "grandcanyon_negative_y.jpg" ) ) ),
                       Surface8u( loadImage( loadResource( "grandcanyon_negative_z.jpg" ) ) ));
    
    mVbo = gl::VboMesh(mMeshHelper.createCube());
    
    try {
        mShader = gl::GlslProg( loadAsset("simpleFresnel.vert"), loadAsset("simpleFresnel.frag"));
    }
    catch ( gl::GlslProgCompileExc err )
    {
        cout << err.what() << endl;
    }
    
    mDiffuseK = 1.33f;
	mRoughness = 0.0010f;
	mIndexOfRefraction = 1.31f;
	mColorAmbient = Color(0.1f, 0.1f, 0.2f);
	mColorDiffuse = Color(0.0f, 0.1f, 0.4f);
	mColorSpecular = Color(1.0f, 1.0f, 1.0f);
	mSpecularAttenuation = 8;
	mTransluscence = 0.05f;
	mLightPosition = Vec3f(300.0f, 300.0f, -7.0f);

    mControls = params::InterfaceGl( getWindow(), "App parameters", toPixels( Vec2i( 200, 400 ) ) );
    
    mControls.addParam( "Diffuse ", &mDiffuseK );
	mControls.addParam( "Roughness ", &mRoughness);
	mControls.addParam( "Refract Index", &mIndexOfRefraction);
	mControls.addParam( "Ambient Color", &mColorAmbient);
	mControls.addParam( "Diffuse Color", &mColorDiffuse);
	mControls.addParam( "Specular Color", &mColorSpecular);
	mControls.addParam( "Specular Attenuation", &mSpecularAttenuation);
	mControls.addParam( "Translucence ", &mTransluscence );
	mControls.addParam( "Light position ", &mLightPosition );
    
}

void cubemapApp::mouseDown( MouseEvent event )
{
    
}

void cubemapApp::update()
{
}

void cubemapApp::draw()
{
    
    glEnable( GL_DEPTH_TEST );
    glDepthMask( GL_TRUE );

	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );

    glEnable( GL_MULTISAMPLE_ARB );

    mCubeMap->bindMulti(0);
    mShader.bind();
    
    mShader.uniform("uK", mDiffuseK);
	mShader.uniform("uRoughness", mRoughness);
	mShader.uniform("uIoR", mIndexOfRefraction);
	mShader.uniform("uAmbient", Vec3f(mColorAmbient.r, mColorAmbient.g, mColorAmbient.b));
	mShader.uniform("uDiffuse", Vec3f(mColorDiffuse.r, mColorDiffuse.g, mColorDiffuse.b));
	mShader.uniform("uSpecular", Vec3f(mColorSpecular.r, mColorSpecular.g, mColorSpecular.b));
	mShader.uniform("uSpecularAttenuation", mSpecularAttenuation);
	mShader.uniform("uTransluscence", mTransluscence);
	mShader.uniform("uLightPosition", mLightPosition);
    mShader.uniform("EnvMap", 0);
    
    glPushMatrix();
    gl::translate( 500.0f, 300, 0.0f );
    gl::drawSphere(Vec3f(0,0,0), 200, 36);
    glPopMatrix();
    
    mShader.unbind();
    mCubeMap->unbind();

//    glDisable( GL_DEPTH_TEST );
//    glDepthMask( GL_FALSE );
    glDisable( GL_MULTISAMPLE_ARB );
    
    glEnable( GL_TEXTURE_2D );
    glActiveTexture(0);
//	glBindTexture(GL_TEXTURE_2D, 0);
    
    gl::color(1,1,1);
    
    // Draw the interface
	mControls.draw();
}

CINDER_APP_NATIVE( cubemapApp, RendererGl )
