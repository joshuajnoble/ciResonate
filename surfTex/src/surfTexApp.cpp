#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Surface.h"
#include "cinder/Utilities.h"
#include "cinder/TriMesh.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class surfTexApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    void keyDown( KeyEvent key );

	gl::Texture mTex;
    Surface8u mSurface;
    
    TriMesh plane;
};

void surfTexApp::setup()
{
    mSurface = Surface(loadImage( loadAsset("image.jpg")));
    mTex = gl::Texture(mSurface);

    plane.appendVertex(Vec3f(0, 0, 0)); // [ (0,0,0) ]
    plane.appendTexCoord(Vec2f(0,0));
    
    plane.appendVertex(Vec3f(600, 0, 0)); // [ (0,0,0), (600,0,0) ]
    plane.appendTexCoord(Vec2f(1.f,0));
    
    plane.appendVertex(Vec3f(600, 600, 0)); // [ (0,0,0), (600,0,0), (600,600,0)]
    plane.appendTexCoord(Vec2f(1.f,0.5f));
    
    plane.appendVertex(Vec3f(0, 600, 0)); // [ (0,0,0), (600,0,0), (600,600,0), (0,600,0)]
    plane.appendTexCoord(Vec2f(0.f,0.5f));
    
    uint indices[6] = {0,1,2,2,3,0};
    plane.appendIndices(&indices[0], 6);
    
    
}

void surfTexApp::mouseDown( MouseEvent event )
{
	Surface::Iter it = mSurface.getIter();
    while( it.line() )
    {
        while( it.pixel() )
        {
            uint8_t r = it.r();
            it.r() = it.g();
            it.g() = it.b();
            it.b() = r;
        }
    }
    
    mTex = gl::Texture( mSurface );
}

void surfTexApp::keyDown (cinder::app::KeyEvent key)
{
    
    if(key.getCode() == KeyEvent::KEY_m)
    {
        Surface copySurf = mSurface.clone();
        
        Surface::ConstIter cit = mSurface.getIter();
        Surface::Iter it = copySurf.getIter();
        
        while(  cit.line() && it.line()  )
        {
            if(cit.y() < mSurface.getHeight() - 1 && cit.x() < mSurface.getWidth() - 1  )
            {
                while( cit.pixel() && it.pixel() )
                {
                    it.b() = cit.r(0,1);
                    it.r() = cit.g(1,1);
                    it.g() = cit.b(0,1);
                }
            }

        }
        mTex = gl::Texture( copySurf );
    }
    
    if(key.getCode() == KeyEvent::KEY_d)
    {
        Surface copySurf = mSurface.clone();
        Surface::ConstIter cit = mSurface.getIter();
        Surface::Iter it = copySurf.getIter();
        while( cit.line() && it.line() )
        {
            while( cit.pixel() && it.pixel() )
            {
                if(cit.x() != 0)
                {
                    it.r() = cit.r() - cit.r(-1,0);
                    it.b() = cit.b() - cit.b(-1,0);
                    it.g() = cit.g() - cit.g(-1,0);
                }
            }
        }
        
        mTex = gl::Texture(copySurf);
    }
    
    if( key.getCode() == KeyEvent::KEY_e)
    {
        Surface copySurf = mSurface.clone();
        Surface::ConstIter cit = mSurface.getIter();
        Surface::Iter it = copySurf.getIter();
        
        while( cit.line() ) {
            it.line();
            while( cit.pixel() ) {
                it.pixel();
                int32_t sumRed = cit.rClamped( 0, -1 ) + cit.rClamped( -1, 0 ) + cit.r() * -4 + cit.rClamped( 1, 0 ) + cit.rClamped( 0, 1 );
                it.r() = constrain<int32_t>( abs( sumRed ), 0, 255 );
                int32_t sumGreen = cit.gClamped( 0, -1 ) + cit.gClamped( -1, 0 ) + cit.g() * -4 + cit.gClamped( 1, 0 ) + cit.gClamped( 0, 1 );
                it.g() = constrain<int32_t>( abs( sumGreen ), 0, 255 );
                int32_t sumBlue = cit.bClamped( 0, -1 ) + cit.bClamped( -1, 0 ) + cit.b() * -4 + cit.bClamped( 1, 0 ) + cit.bClamped( 0, 1 );
                it.b() = constrain<int32_t>( abs( sumBlue ), 0, 255 );
            }
        }
        
        mTex = gl::Texture(copySurf);
    }
    
    /*
     
     LOTS of other matrices, try these out for starters
     
     -1 -1 -1
     -1  9 -1
     -1 -1 -1
     
     1/9 1/9 1/9
     1/9 1/9 1/9
     1/9 1/9 1/9

     1 2 1 2 1 2 1
     3 3 3 3 3 3 3
     1 2 1 2 1 2 1
     3 3 3 3 3 3 3
     1 2 1 2 1 2 1
     
     */
    
    if(key.getCode() == KeyEvent::KEY_s)
    {
        try {
			// load a source image into a Surface to be written later
			Surface srf( copyWindowSurface() );
			
			fs::path pngPath = getSaveFilePath( getHomeDirectory() );
			if( ! pngPath.empty() ) {
				writeImage( pngPath, srf );
			}
		}
		catch( ... ) {
			console() << " Can't save image file for whatever reason " << std::endl;
		}
    }

}

void surfTexApp::update()
{
}

void surfTexApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    //gl::draw( mTex, getWindowBounds());
    
    
    mTex.enableAndBind(); // everything after this gets my texture
    gl::draw(plane);
    mTex.unbind(); // stop using my texture
    

}

CINDER_APP_NATIVE( surfTexApp, RendererGl )
