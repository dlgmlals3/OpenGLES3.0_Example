#include "Pyramid.h"
#include "ProgramManager.h"
#include "Cache.h"

#define VERTEX_SHADER_PRG (char*)"shader/CubeVertex.glsl"
#define FRAGMENT_SHADER_PRG (char*)"shader/CubeFragment.glsl"

Pyramid::Pyramid(Renderer *renderer) {
    TransformObj = renderer->RendererTransform();
    ProgramManagerObj = renderer->RendererProgramManager();
    MapRenderHandler = renderer;
    modelType = PyramidType;
}

Pyramid::~Pyramid(){
    PROGRAM* program;
    if ((program = ProgramManagerObj->Program((char*)"Pyramid"))) {
        ProgramManagerObj->RemoveProgram(program);
    }
}

void Pyramid::InitModel() {
    if (! ( program = ProgramManagerObj->Program( ( char * )"Pyramid" ) ) )
    {
        program = ProgramManagerObj->ProgramInit( ( char * )"Pyramid" );
        ProgramManagerObj->AddProgram( program );
    }

    program->VertexShader = ShaderManager::ShaderInit(VERTEX_SHADER_PRG, GL_VERTEX_SHADER);
    program->FragmentShader = ShaderManager::ShaderInit(FRAGMENT_SHADER_PRG, GL_FRAGMENT_SHADER);

    CACHE *m = reserveCache(VERTEX_SHADER_PRG, true);
    if (m) {
        if (!ShaderManager::ShaderCompile(program->VertexShader,
                                          reinterpret_cast<const char *>(m->buffer),
                                          true)) {
            exit(1);
        }
        free(m);
    }

    m = reserveCache(FRAGMENT_SHADER_PRG, true);
    if (m) {
        if (!ShaderManager::ShaderCompile(program->FragmentShader,
                                         reinterpret_cast<const char*>(m->buffer),
                                         true)) {
            exit(1);
        }
        free(m);
    }
    ProgramManagerObj->ProgramLink(program, true);
    glUseProgram(program->ProgramID);
}

void Pyramid::Update( float t ) { }

void Pyramid:: Resize(int, int) { }

void Pyramid:: Render() {
    LOGI("Pyramid: Render");
    GLfloat pyramidVerts[][3] = {
        // 밑면 (정사각형)
        { -0.10, -0.10, -0.10 }, // 0: 좌측 하단
        {  0.10, -0.10, -0.10 }, // 1: 우측 하단
        {  0.10, -0.10,  0.10 }, // 2: 우측 상단
        { -0.10, -0.10,  0.10 }, // 3: 좌측 상단

        // 꼭짓점 (삼각뿔의 윗부분)
        {  0.00,  0.10,  0.00 }  // 4: 피라미드 꼭짓점
    };

    GLfloat pyramidColors[][3] = {
    {  0.0,  0.0,  0.0 }, // 밑면 정점 0
    {  0.0,  0.0,  1.0 }, // 밑면 정점 1
    {  0.0,  1.0,  0.0 }, // 밑면 정점 2
    {  0.0,  1.0,  1.0 }, // 밑면 정점 3
    {  1.0,  1.0,  0.0 }  // 꼭짓점 (피라미드 상단)
    };

    GLushort pyramidIndices[] = {
        // 밑면 (정사각형을 삼각형 2개로 분할)
        0, 1, 2,
        0, 2, 3,

        // 측면 삼각형
        0, 1, 4,  // 앞면
        1, 2, 4,  // 오른쪽 면
        2, 3, 4,  // 뒷면
        3, 0, 4   // 왼쪽 면
    };


    char attribColor = ProgramManagerObj->ProgramGetVertexAttribLocation(program, (char*)"VertexColor");
    char attribVertex = ProgramManagerObj->ProgramGetVertexAttribLocation(program, (char*)"VertexPosition");

    // 속성이 존재 하지않느면 -1 리턴
    if (attribVertex >= 0) {
        glEnableVertexAttribArray(attribVertex);
        glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 0, pyramidVerts);
    }
    if (attribColor >= 0) {
        glEnableVertexAttribArray(attribColor);
        glVertexAttribPointer(attribColor, 3, GL_FLOAT, GL_FALSE, 0, pyramidColors);
    }

    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_SHORT, pyramidIndices);
    glDisableVertexAttribArray(attribColor);
    glDisableVertexAttribArray(attribVertex);
}

bool Pyramid::useProgram(char* program) {
    return true;
}


void Pyramid::TouchEventDown( float a, float b ) {

}
void Pyramid::TouchEventMove( float a, float b ){

}

void Pyramid::TouchEventRelease( float a, float b ){

}