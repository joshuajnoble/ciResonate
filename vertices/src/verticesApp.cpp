#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/TriMesh.h"
#include "cinder/CinderMath.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define PI 3.14159265358979323846f

class verticesApp : public AppNative {

public:
    
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    void generateCapsule();
    
    TriMesh mesh;
    PolyLine<Vec3f> line;
    vector<Vec3f> points;
    
    float mRadius, mHeight, mNumRings, mNumSegments, mNumSegHeight;
    
};

void verticesApp::setup()
{
    mRadius = 100.0;
    mHeight = 100.0;
    mNumRings = 8;
    mNumSegments = 16;
    mNumSegHeight = 100;
    
    generateCapsule();
}

void verticesApp::generateCapsule()
{
    
    vector<uint> indices;
    
    double fDeltaRingAngle = (M_PI_2 / mNumRings);
    double fDeltaSegAngle = ((M_PI * 2.0) / mNumSegments);
    
    double sphereRatio = mRadius / (2 * mRadius + mHeight);
    double cylinderRatio = mHeight / (2 * mRadius + mHeight);
    int offset = 0;
    // Top half sphere
    
    // Generate the group of rings for the sphere
    for(unsigned int ring = 0; ring <= mNumRings; ring++ )
    {
        double r0 = mRadius * sinf ( ring * fDeltaRingAngle);
        double y0 = mRadius * cosf (ring * fDeltaRingAngle);
        
        // Generate the group of segments for the current ring
        for(unsigned int seg = 0; seg <= mNumSegments; seg++)
        {
            double x0 = r0 * cosf(seg * fDeltaSegAngle);
            double z0 = r0 * sinf(seg * fDeltaSegAngle);
            
            Vec3f p(x0, 0.5f*mHeight + y0, z0);
            Vec3f n(x0, y0, z0);
            mesh.appendVertex(p);
            mesh.appendNormal(n.normalized());
            mesh.appendTexCoord(Vec2f((double) seg / (double) mNumSegments, (double) ring / (double) mNumRings * sphereRatio));
            mesh.appendColorRgb(Colorf(1.0, 1.0, 1.0));
            
            // each vertex (except the last) has six indices pointing to it
            indices.push_back(offset + mNumSegments + 1);
            indices.push_back(offset + mNumSegments);
            indices.push_back(offset);
            indices.push_back(offset + mNumSegments + 1);
            indices.push_back(offset);
            indices.push_back(offset + 1);
            
            offset ++;
        } // end for seg
    } // end for ring
    
    // Cylinder part
    double deltaAngle = ((M_PI * 2.0) / mNumSegments);
    double deltamHeight = mHeight/(double)mNumSegHeight;
    
    for (unsigned short i = 1; i < mNumSegHeight; i++) {
        for (unsigned short j = 0; j<=mNumSegments; j++)
        {
            double x0 = mRadius * cosf(j*deltaAngle);
            double z0 = mRadius * sinf(j*deltaAngle);
            
            Vec3f p(x0, 0.5f*mHeight-i*deltamHeight, z0);
            Vec3f n(x0, 0, z0);
            mesh.appendVertex(p);
            mesh.appendNormal(n.normalized());
            mesh.appendTexCoord(Vec2f(j/(double)mNumSegments, i/(double)mNumSegHeight * cylinderRatio + sphereRatio));
            mesh.appendColorRgb(Colorf(1.0, 1.0, 1.0));

            
            indices.push_back(offset + mNumSegments + 1);
            indices.push_back(offset + mNumSegments);
            indices.push_back(offset);
            indices.push_back(offset + mNumSegments + 1);
            indices.push_back(offset);
            indices.push_back(offset + 1);
            
            offset ++;
        }
    }

    // Bottom half sphere
    
    // Generate the group of rings for the sphere
    for(unsigned int ring = 0; ring <= mNumRings; ring++)
    {
        double r0 = mRadius * sinf (M_PI_2 + ring * fDeltaRingAngle);
        double y0 =  mRadius * cosf (M_PI_2 + ring * fDeltaRingAngle);
        
        // Generate the group of segments for the current ring
        for(unsigned int seg = 0; seg <= mNumSegments; seg++)
        {
            double x0 = r0 * cosf(seg * fDeltaSegAngle);
            double z0 = r0 * sinf(seg * fDeltaSegAngle);
            
            Vec3f p(x0, -0.5f*mHeight + y0, z0);
            Vec3f n(x0, y0, z0);
            mesh.appendVertex(p);
            mesh.appendNormal(n.normalized());
            mesh.appendTexCoord(Vec2f((double) seg / (double) mNumSegments, (double) ring / (double) mNumRings*sphereRatio + cylinderRatio + sphereRatio));
            mesh.appendColorRgb(Colorf(1.0, 1.0, 1.0));

            
            if (ring != mNumRings) 
            {
                // each vertex (except the last) has six indices pointing to it
                indices.push_back(offset + mNumSegments + 1);
                indices.push_back(offset + mNumSegments);
                indices.push_back(offset);
                indices.push_back(offset + mNumSegments + 1);
                indices.push_back(offset);
                indices.push_back(offset + 1);
            }
            offset ++;
        } // end for seg
    } // end for ring
    
    mesh.appendIndices( &indices[0], indices.size());
	
    cout << mesh.getVertices().size() << " " << mesh.getNormals().size() << endl;
}

void verticesApp::mouseDown( MouseEvent event )
{
}

void verticesApp::update()
{
}

void verticesApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    gl::pushMatrices();
    gl::translate(300, 300);
    //gl::color(255, 255, 255);
    gl::draw(mesh);
    gl::popMatrices();
}

CINDER_APP_NATIVE( verticesApp, RendererGl )
