#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Surface.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"
#include "cinder/qtime/QuickTime.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class videoMeshApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
    qtime::MovieGl mMovie;
    gl::VboMesh mVbo;
    
    vector<Vec3f> positions;
    vector<float> rotations;
    vector<Vec2f> texCoords;
    
    gl::Texture texCoordDataTex;
    Surface dataSurface, sourceSurface;
    Rand mRand;
    
    gl::GlslProg mShader;
    qtime::MovieLoader loader;
    
    bool mMovieIsReady;
};

void videoMeshApp::setup()
{
    
    loader = qtime::MovieLoader( Url( "http://movies.apple.com/movies/us/hd_gallery/gl1800/480p/bbc_earth_m480p.mov" ) );
    mMovie = qtime::MovieGl( loader );
    mMovie.setLoop();
    mMovie.play();
    
    sourceSurface = Surface(640,480,false);
    dataSurface = Surface(640,480,false);
    gl::Texture::Format f;
    f.setInternalFormat(GL_TEXTURE_RECTANGLE_ARB);
    texCoordDataTex = gl::Texture(640,480,f);
    
    mShader = gl::GlslProg( loadAsset("resample.vs"), loadAsset("resample.fs") );
    mMovieIsReady = false;
    
    TriMesh plane;
    plane.appendVertex(Vec3f(0, 0, 0));
    plane.appendTexCoord(Vec2f(0,0));
    plane.appendVertex(Vec3f(20, 0, 0));
    plane.appendTexCoord(Vec2f(1,0));
    plane.appendVertex(Vec3f(20, 20, 0));
    plane.appendTexCoord(Vec2f(1,1));
    plane.appendVertex(Vec3f(0, 20, 0));
    plane.appendTexCoord(Vec2f(0,1));
    
    uint indices[6] = {0,1,2,2,3,0};
    plane.appendIndices(&indices[0], 6);
    
    gl::VboMesh::Layout layout;
    mVbo = gl::VboMesh(plane);
    
    for( int x = 0; x < 20; x++ ) {
        for( int y = 0; y < 20; y++ ) {
            positions.push_back(Vec3f( x * 32, y * 24, 0));
            rotations.push_back(mRand.randFloat(0, 360));
        }
    }
    
    Surface::Iter it = sourceSurface.getIter();
    int i = 0, j = 0;
    while( it.line() ) {
        j = 0;
        while( it.pixel() )
        {
            it.r() = 255.f * (float(j)/640.f);
            it.g() = 255.f * (float(i)/480.f);
            it.b() = 0;
            j++;
        }
        i++;
    }
    dataSurface.copyFrom(sourceSurface, Area(0,0,640,480));
    texCoordDataTex = gl::Texture(dataSurface);
}

void videoMeshApp::mouseDown( MouseEvent event )
{
    
    for( int i = 0; i < 16; i++)
    {
        int startX = mRand.randInt(0, 640);
        int startY = mRand.randInt(0, 480);
        int width = mRand.randInt(60, 160);
        int height = mRand.randInt(20, 80);
        
        Vec2i offSet(mRand.randInt(0, 640),mRand.randInt(0, 480));
        
        dataSurface.copyFrom(sourceSurface, Area( startX, startY, width, height ), offSet );
    }
    
    texCoordDataTex = gl::Texture(dataSurface);
}

void videoMeshApp::update()
{
    
    if(loader.checkLoaded()) {
        mMovieIsReady = true;
    }
    
    for( int i = 0; i < positions.size(); i++ )
    {
        positions[i] += Vec3f( mRand.posNegFloat(-0.2,0.2), mRand.posNegFloat(-0.2,0.2), mRand.posNegFloat(-0.2,0.2));
    }
    
    for( int i = 0; i < rotations.size(); i++ )
    {
        rotations[i] += mRand.randFloat(0, 1.0);
    }

//    Surface::Iter it = dataSurface.getIter();
//    int i = 0, j = 0;
//    while( it.line() ) {
//        j = 0;
//        while( it.pixel() )
//        {
//            it.r() = 255.f * (float(j)/640.f);
//            it.g() = 255.f * (float(i)/480.f);
//            it.b() = 0;
//            j++;
//        }
//        i++;
//    }
//    
//    texCoordDataTex = gl::Texture(dataSurface);
}

void videoMeshApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    if(!mMovieIsReady || !mMovie.getTexture()) {
        return; // wait for movie to be loaded
    }
    
    texCoordDataTex.enableAndBind();
    mMovie.getTexture().bind(1);
    
    mShader.bind();
    
    mShader.uniform("dataTex", 0);
    mShader.uniform("tex", 1);
    
    vector<Vec3f>::iterator pit = positions.begin();
    vector<float>::iterator rit = rotations.begin();
    while( pit != positions.end() && rit != rotations.end())
    {
        
        gl::pushMatrices();
        
        Vec3f r(*rit, *rit, *rit);
        gl::translate(*pit); // do our transform
        gl::rotate( r ); // do our rotations
        gl::draw(mVbo); // draw the mesh
        
        gl::popMatrices();
        
        ++pit;
        ++rit;
    }
    
    mShader.unbind();
    
    texCoordDataTex.unbind(0);
    mMovie.getTexture().unbind(1);

    
}

CINDER_APP_NATIVE( videoMeshApp, RendererGl )
