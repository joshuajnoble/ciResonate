#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "CinderOpenCV.h"
#include "cinder/Capture.h"
#include "cinder/ImageIO.h"
#include "cinder/Camera.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static const int APP_WIDTH	= 1280;
static const int APP_HEIGHT	= 960;
static const int CAM_RES	= 8;
static const int CAM_WIDTH	= APP_WIDTH/CAM_RES;
static const int CAM_HEIGHT	= APP_HEIGHT/CAM_RES;
static const int CUBE_RES	= 16;

struct BoxVert {
	ci::Vec3f vertex;
	ci::Vec3f normal;
	ci::Vec4f color;
};

struct Box {
	Box( Vec3f pos ){
		mAxis		= Vec3f::yAxis();
		mAxisDest	= Vec3f::zAxis();
		mPos		= pos;
		mSize		= 20.0f;
		mRotAngle	= 0.0f;
		mRotVel		= 0.01f;
	}
	
	void update( float xOff, float yOff ){
		Vec2f dir = Vec2f( xOff + 0.00001f, yOff - 0.00001f );
		float dist = dir.length();
		
		if( dist > 0.4f ){
			mRotVel		+= dist * 0.5;
			mPerp		= Vec3f( -dir.y, dir.x, 0.0f ).normalized();
			mAxisDest	-= ( mAxisDest - mPerp ) * 0.15f;
			mAxis		-= ( mAxis - mAxisDest ) * 0.15f;
		}
		
		mRotAngle	+= mRotVel;
		mRotAngle	= fmod( mRotAngle, 180.0f );
		mRotVel		*= 0.97f;
		mRotAngle	-= ( mRotAngle ) * 0.1f;
		
		mMatrix.setToIdentity();
		mMatrix.translate( mPos );
		mMatrix.rotate( mAxis, toRadians( mRotAngle ) );
		mMatrix.scale( Vec3f( 4.0f, 4.0f, mSize ) );
		
		mNormMatrix.setToIdentity();
		mNormMatrix.rotate( mAxis, toRadians( mRotAngle ) );
	}
	
	void adjustSize( float offset ){
		mSize = constrain( mSize + offset, 5.0f, 200.0f );
	}
	
	void addToVbo( Vec3f *dest, const Vec3f &p ){
		*dest++ = mMatrix * p;
	}
	
	Vec3f mPos;
	Vec3f mAxis, mPerp, mAxisDest;
	float mRotAngle;
	float mRotVel;
	float mSize;
	Matrix44f mMatrix;
	Matrix44f mNormMatrix;
};


class farnebackCubesFastApp : public AppBasic {
public:
	void prepareSettings( Settings *settings );
	void setup();
	void mouseWheel( MouseEvent event );
	void update();
	void updateVbos();
	void draw();
	
	// CAMERA
	CameraPersp		mCam;
	Vec3f			mEye, mCenter;
	Matrix44f		mMvpMatrix;
	
	// CAPTURE
    Capture			mCapture;
	Surface			mCaptureSurface;
	gl::Texture		mCaptureTex;
	
	// SHADERS
	gl::GlslProg	mShader;
	gl::GlslProg	mBoxShader;
	
	// BOXES
	vector<Box>			mBoxes;
	int					mXBoxRes;
	int					mYBoxRes;
	int					mNumBoxes;
	
	int					mTotalBoxVerts;
    int					mPrevTotalBoxVerts;
    BoxVert				*mBoxVerts;
	
	// OPENCV
    gl::Texture		mHsvTexture;
	gl::Texture		mTextureX, mTextureY;
    cv::Mat			mPrevFrame, mCurrentFrame, mFlowFrame;
    vector<cv::Mat>	mConvertedFlow; // this will hold our split farneback flow
    Surface32f		mFlowSurfaceX, mFlowSurfaceY, mHsvSurface;
	
    float	pyrScale;
    int		levels;
    int		winSize;
    int		iterations;
    int		polyN;
    float	polySigma;
};

void farnebackCubesFastApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( APP_WIDTH, APP_HEIGHT );
}

void farnebackCubesFastApp::setup()
{
	// CAMERA
	mCenter			= Vec3f( getWindowWidth() * 0.5f, getWindowHeight() * 0.5f, 0.0f );
	mEye			= mCenter + Vec3f( 0.0f, 0.0f, 820.0f );
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), 1.0f, 1000.0f );
	mCam.lookAt( mEye, mCenter, -Vec3f::yAxis() );
	mMvpMatrix = mCam.getProjectionMatrix() * mCam.getModelViewMatrix();
	
	// CAPTURE
	mCaptureTex		= gl::Texture( CAM_WIDTH, CAM_HEIGHT );
	mCaptureSurface = Surface( CAM_WIDTH, CAM_HEIGHT, false );
    mCapture		= Capture( CAM_WIDTH, CAM_HEIGHT );
    mCapture.start();
	
	// TEXTURES
	gl::Texture::Format format;
	format.setInternalFormat( GL_RGBA32F_ARB );
	mHsvTexture		= gl::Texture( CAM_WIDTH, CAM_HEIGHT, format );
	mTextureX		= gl::Texture( CAM_WIDTH, CAM_HEIGHT, format );
	mTextureY		= gl::Texture( CAM_WIDTH, CAM_HEIGHT, format );
	
	// SHADERS
	try {
		mShader		= gl::GlslProg( loadAsset( "shader.vert" ), loadAsset( "shader.frag" ) );
		mBoxShader	= gl::GlslProg( loadAsset( "box.vert" ), loadAsset( "box.frag" ) );
	} catch( gl::GlslProgCompileExc e ) {
		std::cout << e.what() << std::endl;
		quit();
	}
	
	// BOXES
	mXBoxRes	= APP_WIDTH/CUBE_RES;
	mYBoxRes	= APP_HEIGHT/CUBE_RES;
	mNumBoxes	= mXBoxRes * mYBoxRes;
	for( int y=0; y<mYBoxRes; y++ ){
		for( int x=0; x<mXBoxRes; x++ ){
			Vec3f pos = Vec3f( x * CUBE_RES, y * CUBE_RES, 0.0f );
			mBoxes.push_back( Box( pos ) );
		}
	}
	
	mPrevTotalBoxVerts	= -1;
	mTotalBoxVerts		= 0;
    mBoxVerts			= NULL;
	
    // OPENCV PARAMS
	pyrScale	= 0.5;
    levels		= 1;
    winSize		= 7;
    iterations	= 2;
    polyN		= 3;
    polySigma	= 1.1;
}

void farnebackCubesFastApp::mouseWheel( MouseEvent event )
{
	for( int i=0; i<mBoxes.size(); i++ ){
		mBoxes[i].adjustSize( event.getWheelIncrement() );
	}
}

void farnebackCubesFastApp::update()
{	
    if( mCapture.checkNewFrame() )
	{
		mCaptureSurface = mCapture.getSurface();
		mCaptureTex		= gl::Texture( mCaptureSurface );
		mCurrentFrame	= toOcv( mCaptureSurface, CV_8U ) ;
		
		if(mPrevFrame.size().width > 0) {
			cv::calcOpticalFlowFarneback(mPrevFrame, mCurrentFrame, mFlowFrame, pyrScale, levels, winSize, iterations, polyN, polySigma, cv::OPTFLOW_FARNEBACK_GAUSSIAN); 
			
			cv::split(mFlowFrame, mConvertedFlow);
			
			mFlowSurfaceX	= fromOcv(mConvertedFlow[0]);
			mFlowSurfaceY	= fromOcv(mConvertedFlow[1]);
			
			mTextureX		= gl::Texture( mFlowSurfaceX );
			mTextureY		= gl::Texture( mFlowSurfaceY );
		}
		
		mPrevFrame = mCurrentFrame;
	}
	
	// BOXES
	if( mFlowSurfaceX && mFlowSurfaceY ){
		for( int i=0; i<mBoxes.size(); i++ ){
			Vec2i v = mBoxes[i].mPos.xy()/CAM_RES;
			ColorAf cx = mFlowSurfaceX.getPixel( v );
			ColorAf cy = mFlowSurfaceY.getPixel( v );
			mBoxes[i].update( cx.r, cy.r );
		}
	}
	
	updateVbos();
}

void farnebackCubesFastApp::updateVbos()
{
	static Vec3f boxVerts[8] = {
		Vec3f(-1,-1,-1 ), Vec3f(-1,-1, 1 ),
		Vec3f( 1,-1, 1 ), Vec3f( 1,-1,-1 ),
		Vec3f(-1, 1,-1 ), Vec3f(-1, 1, 1 ),
		Vec3f( 1, 1, 1 ), Vec3f( 1, 1,-1 ) };
	
	static Vec3f boxNorms[6] = {
		Vec3f( 0, -1, 0 ), Vec3f( -1, 0, 0 ), Vec3f( 0, 0, -1 ),
		Vec3f( 1, 0, 0 ), Vec3f( 0, 0, 1 ), Vec3f( 0, 1, 0 ) };
	
	static GLuint triIndices[12][3] = {
		{0,1,2}, {0,2,3}, {0,4,5}, {0,5,1}, {3,7,4}, {3,4,0},
		{2,6,7}, {2,7,3}, {1,5,6}, {1,6,2}, {4,5,6}, {4,6,7} };
	
	mTotalBoxVerts = mNumBoxes * 36;	// 12 triangles per box * 3 verts per triangle
    if (mTotalBoxVerts != mPrevTotalBoxVerts) {
        if (mBoxVerts != NULL) {
            delete[] mBoxVerts;
        }
        mBoxVerts = new BoxVert[mTotalBoxVerts];
        mPrevTotalBoxVerts = mTotalBoxVerts;
    }
	
	int boxCount	= 0;
	int vIndex		= 0;
	for( int y=0; y<mYBoxRes; y++ ){
		for( int x=0; x<mXBoxRes; x++ ){
			Box *box		= &mBoxes[boxCount];
			Vec2i cIndex	= Vec2i( x * CUBE_RES, y * CUBE_RES )/CAM_RES;
			ColorA c		= mCaptureSurface.getPixel( cIndex );
			Vec4f col		= Vec4f( c );
			
			// MULTIPLY THE MATRIX AND BOXVERTS
			// (so we only do it 8 times instead of 36 times
			Vec3f newBoxVerts[8];
			for( int i=0; i<8; i++ ){
				newBoxVerts[i] = box->mMatrix * boxVerts[i];
			}
			// MULTIPLY THE NORM-MATRIX AND NORMALS
			// (so we only do it 6 times instead of 12
			Vec3f newBoxNorms[6];
			for( int i=0; i<6; i++ ){
				newBoxNorms[i] = box->mNormMatrix * boxNorms[i];
			}
			
			
			for( int i=0; i<12; i++ ){
				Vec3f norm = newBoxNorms[i/2];
				mBoxVerts[vIndex].vertex = newBoxVerts[triIndices[i][0]];
				mBoxVerts[vIndex].normal = norm;
				mBoxVerts[vIndex].color	 = col;
				vIndex ++;
				
				mBoxVerts[vIndex].vertex = newBoxVerts[triIndices[i][1]];
				mBoxVerts[vIndex].normal = norm;
				mBoxVerts[vIndex].color	 = col;
				vIndex ++;
				
				mBoxVerts[vIndex].vertex = newBoxVerts[triIndices[i][2]];
				mBoxVerts[vIndex].normal = norm;
				mBoxVerts[vIndex].color	 = col;
				vIndex ++;
				
			}
			
			boxCount ++;
		}
	}
}

void farnebackCubesFastApp::draw()
{	
	gl::clear( ColorA( 0, 0, 0, 0 ), true );
	gl::color( ColorA( 1, 1, 1, 1 ) );
	
	gl::setMatrices( mCam );
	gl::setViewport( getWindowBounds() );
	
	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::enableAlphaBlending();
	
	mCaptureTex.bind();
	mBoxShader.bind();
	mBoxShader.uniform( "captureTex", 0 );
	mBoxShader.uniform( "mvpMatrix", mMvpMatrix );
	mBoxShader.uniform( "screenSize", Vec2f( getWindowSize() ) );
	mBoxShader.uniform( "eyePos", mEye );
	
	if( mTotalBoxVerts > 0 ){
		glEnableClientState( GL_VERTEX_ARRAY );
		glEnableClientState( GL_NORMAL_ARRAY );
		glEnableClientState( GL_COLOR_ARRAY );
		glVertexPointer( 3, GL_FLOAT, sizeof(BoxVert), mBoxVerts );
		glNormalPointer( GL_FLOAT, sizeof(BoxVert), &mBoxVerts[0].normal );
		glColorPointer( 4, GL_FLOAT, sizeof(BoxVert), &mBoxVerts[0].color );
		
		glDrawArrays( GL_TRIANGLES, 0, mTotalBoxVerts );
		
		glDisableClientState( GL_VERTEX_ARRAY );
		glDisableClientState( GL_NORMAL_ARRAY );
		glDisableClientState( GL_COLOR_ARRAY );
	}
	
	gl::disableDepthRead();
	mBoxShader.unbind();
	
	if( mCaptureTex && mFlowSurfaceX && mFlowSurfaceY ){
		gl::draw( mCaptureTex );
		gl::draw( gl::Texture( mFlowSurfaceX ), Vec2f( 0.0f, CAM_HEIGHT ) );
		gl::draw( gl::Texture( mFlowSurfaceY ), Vec2f( 0.0f, CAM_HEIGHT*2 ) );
	}
	
	if( getElapsedFrames() % 60 == 59 )
		std::cout << getAverageFps() << std::endl;
}

CINDER_APP_NATIVE( farnebackCubesFastApp, RendererGl )
