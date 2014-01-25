#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/TriMesh.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Capture.h"
#include "cinder/Triangulate.h"
#include "cinder/Sphere.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class cameras2App : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );
    void mouseDrag( MouseEvent event );
	void update();
	void draw();
    void planeMesh(float width, float height, int columns, int rows, TriMesh& mesh);
    
    MayaCamUI mMayaCam;
    
    TriMesh mMesh;
    CaptureRef mCapture;
    
};

void cameras2App::setup()
{
    
    planeMesh(1024, 768, 320, 240, mMesh);
    
    CameraPersp cam;
    cam.setEyePoint(Vec3f(5, 10, 10));
    cam.setCenterOfInterestPoint(Vec3f::zero());
    cam.setPerspective(60.0, getWindowAspectRatio(), 1.0, 10000.0);
    mMayaCam.setCurrentCam( cam );
    
    // set up the camera
	CameraPersp initialCam;
    Sphere boundingSphere = Sphere::calculateBoundingSphere( mMesh.getVertices() );
	mMayaCam.setCurrentCam( mMayaCam.getCamera().getFrameSphere( boundingSphere, 100 ) );
	//mCam.setCurrentCam( initialCam );
    
    mCapture = Capture::create(320, 240);
    mCapture->start();
    
}

void cameras2App::mouseDown( MouseEvent event )
{
    mMayaCam.mouseDown(event.getPos());
}

void cameras2App::mouseDrag( MouseEvent event )
{
    mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void cameras2App::update()
{
    // use pixel darkness to extrude a mesh, rotate/transform in 3d space
    if(mCapture->checkNewFrame()) {
        
        Surface surf = mCapture->getSurface();
        Surface::Iter iter = surf.getIter();
        
        int i = 0;
        
        while( iter.line() ) {
            while( iter.pixel() ) {
                
                float bright = (iter.r() + iter.g() + iter.b()) / 3.f;
                mMesh.getVertices().at( i ).z = bright * 2.f;
                mMesh.getColorsRGB().at(i).set(iter.r()/255.f, iter.g()/255.f, iter.b()/255.f);
                i++;
            }
        }
        
    }
}

void cameras2App::planeMesh(float width, float height, int columns, int rows, TriMesh& mesh)
{
    
    ////....
    Vec3f vert;
    Vec3f normal(0, 0, 1); // always facing forward //
    Vec2f texcoord;
    
    // the origin of the plane is the center //
    float halfW = width/2.f;
    float halfH = height/2.f;
    // add the vertexes //
    for(int iy = 0; iy < rows; iy++) {
        for(int ix = 0; ix < columns; ix++) {
            
            // normalized tex coords //
            texcoord.x = ((float)ix/((float)columns-1.f));
            texcoord.y = ((float)iy/((float)rows-1.f));
            
            vert.x = texcoord.x * width - halfW;
            vert.y = texcoord.y * height - halfH;
            vert.z = 100;
            
            mesh.appendVertex(vert);
            mesh.appendTexCoord(texcoord);
            mesh.appendNormal(normal);
            mesh.appendColorRgb(Colorf(1.f, 1.f, 1.f));
        }
    }
                              
    uint *indices = new uint[ (rows-1) * (columns-1) * 6 ];
    int i = 0;
    
    for(int y = 0; y < rows-1; y++) {
        for(int x = 0; x < columns-1; x++) {
            // first triangle //
            indices[i] = ((y)*columns + x);
            indices[i+1] = ((y)*columns + x+1);
            indices[i+2] = ((y+1)*columns + x);
            
            // second triangle //
            indices[i+3] = ((y)*columns + x+1);
            indices[i+4] = ((y+1)*columns + x+1);
            indices[i+5] = ((y+1)*columns + x);
            
            i+=6;
        }
    }
    
    mesh.appendIndices(&indices[0], ((rows-1) * (columns-1) * 6));
    
    delete[] indices;

    //return mesh;
}

void cameras2App::draw()
{
    
    gl::clear();
    
    gl::setMatrices( mMayaCam.getCamera() );
    
    //  (note: apply transformations to the model)
	gl::pushModelView();
        gl::draw( mMesh );
	gl::popModelView();
}

CINDER_APP_NATIVE( cameras2App, RendererGl )
