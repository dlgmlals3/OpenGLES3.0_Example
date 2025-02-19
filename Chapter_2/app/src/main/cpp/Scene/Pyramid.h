//
// Created by user on 2025-02-20.
//

#include "Constant.h"
#include "Renderer.h"

#ifndef CHAPTER_2_PYRAMID_H
#define CHAPTER_2_PYRAMID_H

#endif //CHAPTER_2_PYRAMID_H

class Pyramid : public Model {
public:
    Pyramid(Renderer*);
    ~Pyramid();
    void InitModel();
    void Update( float t );
    void Render();
    void Resize(int, int);
    ModelType GetModelType() { return modelType; }
    bool useProgram(char* program);
    void setStates(){}

    virtual void TouchEventDown( float a, float b );
    virtual void TouchEventMove( float a, float b );
    virtual void TouchEventRelease( float a, float b );

private:
    ModelType modelType;
};