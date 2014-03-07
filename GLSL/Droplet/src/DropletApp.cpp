#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/TriMesh.h"
#include "cinder/gl/Vbo.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DropletApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();

    gl::GlslProg force, advec, div, show, p;
    gl::Fbo mFbo1, mFbo2; // we're going to ping-pong with these
    
    int frames;
    float n;
    Surface mSurf;
    
    gl::VboMeshRef         shadowPlane;
    TriMesh plane;
    
    int iterations;
    
};

void DropletApp::setup()
{
	force = gl::GlslProg( loadAsset("shader.vs"), loadAsset("force.fs"));
	advec = gl::GlslProg( loadAsset("shader.vs"), loadAsset("advec.fs"));
	div = gl::GlslProg( loadAsset("shader.vs"), loadAsset("div.fs"));
	show = gl::GlslProg( loadAsset("shader.vs"), loadAsset("show.fs"));
	p = gl::GlslProg( loadAsset("shader.vs"), loadAsset("p.fs"));

    frames = 0;
    
    mSurf = Surface(512, 512, true);
    
	// make this and push it
	float h = 2.f/n;
    float T = 0;
    
	for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            int x = h*(j-n/2);
            int y = h*(i-n/2) - .75;
            if (x*x + y*y > .05) {
                T = 0; }
            else {
                T= -2;
            }
            mSurf.setPixel(Vec2f(i,j), ColorAf(0.f,0.f,T,0.f));
            //}
        }
    }

    float size = 3000;
    
    plane.appendVertex(Vec3f(size, -1,-size));
    plane.appendColorRgba(ColorA(255,255,255,255));
    plane.appendNormal(Vec3f(.0f, 1.0f, 0.0f));
    plane.appendVertex(Vec3f(-size, -1,-size));
    plane.appendColorRgba(ColorA(255,255,255,255));
    plane.appendNormal(Vec3f(.0f, 1.0f, 0.0f));
    plane.appendVertex(Vec3f(-size, -1, size));
    plane.appendColorRgba(ColorA(255,255,255,255));
    plane.appendNormal(Vec3f(.0f, 1.0f, 0.0f));
    plane.appendVertex(Vec3f(size, -1, size));
    plane.appendColorRgba(ColorA(255,255,255,255));
    plane.appendNormal(Vec3f(.0f, 1.0f, 0.0f));
    
    uint indices[6] = {0,1,2,2,3,0};
    plane.appendIndices(&indices[0], 6);
    
    gl::VboMesh::Layout layout;
    shadowPlane = gl::VboMesh::create(plane);
}

void DropletApp::mouseDown( MouseEvent event )
{
}

void DropletApp::update()
{
	/*var bu = v.valueOf();
	gl.useProgram(prog_force); // c = dt*b/2
	gl.uniform1f(gl.getUniformLocation(prog_force, "c"), .001*.5*bu );*/

}

void DropletApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    mFbo2.bindFramebuffer();
    force.bind();
    gl::draw(shadowPlane);
    force.unbind();
    mFbo2.unbindFramebuffer();
    
    mFbo1.bindFramebuffer();
    advec.bind();
    gl::draw(shadowPlane);
    advec.unbind();
    mFbo1.unbindFramebuffer();
    
    p.bind();
	for(int i = 0; i < iterations; i++){
        p.uniform("sampleLocation", 0);//gl.uniform1i(sampLoc, 0);
        mFbo2.bindFramebuffer();
        //gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
        mFbo2.unbindFramebuffer();
        
        //gl.uniform1i(sampLoc, 1);
        //gl.bindFramebuffer(gl.FRAMEBUFFER, FBO);
        //gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
        gl::draw(shadowPlane);
        p.uniform("sampleLocation", 1);
        mFbo1.bindFramebuffer();
        //gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
        mFbo1.unbindFramebuffer();
	}
    p.uniform("sampLoc", 0);
	mFbo2.bindFramebuffer();//gl.bindFramebuffer(gl.FRAMEBUFFER, FBO1);
	//gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
    gl::draw(shadowPlane);
    p.unbind();
    
    mFbo1.bindFramebuffer();
	div.bind();//gl.useProgram(prog_div);
	//gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
    gl::draw(shadowPlane);
    div.unbind();
    mFbo1.unbindFramebuffer();

    
    show.bind();
    gl::draw(shadowPlane);
	//gl.bindFramebuffer(gl.FRAMEBUFFER, null);
	//gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
    show.unbind();
    
    //frames++;
}

CINDER_APP_NATIVE( DropletApp, RendererGl )
