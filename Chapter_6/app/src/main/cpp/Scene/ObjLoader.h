#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "Model.h"
#include "glutils.h"
#include "Renderer.h"
#include <string>
#include "WaveFrontAssetOBJ.h"
#include <queue>

#define GL_CHECK(stmt)                            \
    do {                                          \
        stmt;                                     \
        GLenum err = glGetError();                \
        if (err != GL_NO_ERROR) {                 \
            printf("OpenGL error %x at %s:%d\n",  \
                   err, __FILE__, __LINE__);      \
        }                                         \
    } while (0)

using namespace std;
class ObjLoader : public Model
{

private:
    void TouchEventDown( float x, float y );
    void TouchEventDoubleClick( float x, float y );
	void TouchEventMove( float x, float y );
	void TouchEventRelease( float x, float y );

public:
    // Constructor for ObjLoader
    ObjLoader( Renderer* parent = 0);

    // Destructor for ObjLoader
    ~ObjLoader();

    // Initialize our Model class
    void InitModel();

    // Render the Model class
    void Render();

private:
    // Load the mesh model
    void LoadMesh();
    
    // Switch the mesh model
    void SwitchMesh();
    
    // Release the mesh model resources
    void ReleaseMeshResources();
    void InitMultiLight();

    void debugCode() {
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            LOGE("Error 0x%x", error);
            return;
        }
        LOGE("NO Error 0x%x", error);
    }
    GLuint OBJ_VAO_Id;
    GLuint vertexBuffer;
    OBJMesh waveFrontObjectModel;
    Mesh* objMeshModel;

    GLvoid* offset;
    GLvoid* offsetTexCoord;
    int stride;

    char MVP;
    int ModelNumber;
    char MV;
    GLint NormalMatrix;
    // Parse the wavefront OBJ mesh
    unsigned char RenderPrimitive;
    // Number of vertex Indices
    int IndexCount;

    GLint timer;
    float timeUpdate = 0.0, lastUpdate = 0.0;
    GLint toggleWobble;
    bool toggle = true;
};

#endif // OBJLOADER_H
