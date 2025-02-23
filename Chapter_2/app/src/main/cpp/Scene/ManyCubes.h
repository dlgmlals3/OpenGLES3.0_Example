#ifndef CUBE_MANY_H
#define CUBE_MANY_H

#include "Model.h"
#include "glutils.h"
#include "Renderer.h"

class ManyCubes : public Model {
private:
    char mvp;
    char attribVertex;
    char attribColor;
    bool Animate = true;
    int dimension = 10;
    float distance = 5.0;
    bool op = true;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    int size = 0;
    bool toogle = false;
public:
    ManyCubes(Renderer *parent = 0);

    ~ManyCubes();

    void InitModel();

    void Render();

    void RenderCube();

    void RenderCubeOfCubes();

    void RenderPlus();

    void BehindCubes();

    void TouchEventDown(float a, float b);

    void Culling();

    void InitBufferObject();
};

#endif // CUBE_H
