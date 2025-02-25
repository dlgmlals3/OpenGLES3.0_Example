#ifndef INSTANCE_CUBE
#define INSTANCE_CUBE_H

#include "Model.h"
#include "glutils.h"
#include "Renderer.h"
#include <string>

class InstanceCube : public Model
{
public:
    InstanceCube( Renderer* parent = 0);
    ~InstanceCube();

    void InitModel();
    
    void Render();
    void RenderOrigin();
    void RenderRandomCube();

    void RenderCube();
    void TouchEventDown( float x, float y );
    void CheckGLAPI(std::string str) {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR){
            LOGI("error :%d %s" ,err, str.c_str());
        }
    }

private:
    GLuint vId;
    GLuint iId;
    GLuint matrixId;
    GLuint Vertex_VAO_Id;
    GLuint mvpLoc;
    int size;

    const int dimension    = 10;
    float distance   = 5.0;
    const int OffsetArraySize = 10;

    // Namespace used
    bool Animate = true;
    GLfloat  cubeVerts[8][3] = {
            -1, -1, 1 , // V0
            -1, 1,  1 , // V1
            1,  1, 1 ,  // V2
            1,  -1,  1 ,// V3
            -1, -1, -1 ,// V4
            -1, 1,  -1 ,// V5
            1,  1, -1 , // V6
            1,  -1,  -1 // V7
    };

    GLfloat  cubeColors[8][3] = {
            {  0.0,  0.0,  0.0 }, //0
            {  0.0,  0.0,  1.0 }, //1
            {  0.0,  1.0,  0.0 }, //2
            {  0.0,  1.0,  1.0 }, //3
            {  1.0,  0.0,  0.0 }, //4
            {  1.0,  0.0,  1.0 }, //5
            {  1.0,  1.0,  0.0 }, //6
            {  1.0,  1.0,  1.0 }, //7
    };

    GLushort cubeIndices[36] = {
            0,3,1, 3,2,1,
            7,4,6, 4,5,6,
            4,0,5, 0,1,5,
            3,7,2, 7,6,2,
            1,2,5, 2,6,5,
            3,0,7, 0,4,7
    };
};

#endif // CUBE_H
