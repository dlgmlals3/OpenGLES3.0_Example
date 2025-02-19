#ifndef CUBE_MANY_H
#define CUBE_MANY_H

#include "Model.h"
#include "glutils.h"
#include "Renderer.h"

class ManyCubes : public Model
{
private:
    char mvp;
    char attribVertex;
    char attribColor;
    bool Animate            = true;
    int dimension    = 10;
    float distance   = 5.0;
    bool op          = true;
    GLuint vId              = 0;
    GLuint iId              = 0;
    int size                = 0;
    bool toogle = false;
public:
    ManyCubes( Renderer* parent = 0);
    ~ManyCubes();

    void InitModel();
    
    void Render();
    void RenderCube();
    void RenderCubeOfCubes();
    void RenderPlus();
    void BehindCubes();
    void TouchEventDown( float a, float b );
};

#endif // CUBE_H
