#include "Cube.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "Cache.h"
//#include "Transform.h"
#include "constant.h"
#include "ManyCubes.h"

#define VERTEX_SHADER_PRG			( char * )"shader/BlueTriangleVertex.glsl"
#define FRAGMENT_SHADER_PRG			( char * )"shader/BlueTriangleFragment.glsl"

// Namespace used
using std::ifstream;
using std::ostringstream;

// Global Object Declaration
GLfloat  cubeVerts[][3] = {
	-1, -1, 1 , // V0
	-1, 1,  1 , // V1
	1,  1, 1 ,  // V2
	1,  -1,  1 ,// V3
	-1, -1, -1 ,// V4
	-1, 1,  -1 ,// V5
	1,  1, -1 , // V6
	1,  -1,  -1 // V7
};

GLfloat  cubeColors[][3] = {
    {  0.0,  0.0,  0.0 }, //0
    {  0.0,  0.0,  1.0 }, //1
    {  0.0,  1.0,  0.0 }, //2
    {  0.0,  1.0,  1.0 }, //3
    {  1.0,  0.0,  0.0 }, //4
    {  1.0,  0.0,  1.0 }, //5
    {  1.0,  1.0,  0.0 }, //6
    {  1.0,  1.0,  1.0 }, //7
};

GLushort cubeIndices[] = {0,3,1, 3,2,1,   // 36 of indices
                 7,4,6, 4,5,6,
                 4,0,5, 0,1,5,
                 3,7,2, 7,6,2,
                 1,2,5, 2,6,5,
                 3,0,7, 0,4,7};


ManyCubes::ManyCubes( Renderer* parent )
{
	if (!parent)
		return;

	MapRenderHandler	= parent;
	ProgramManagerObj	= parent->RendererProgramManager();
	TransformObj		= parent->RendererTransform();
	modelType 			= CubeType;
	size = 24 * sizeof(float);
}

ManyCubes::~ManyCubes()
{
	PROGRAM* program = NULL;
	if ( program = ProgramManagerObj->Program( ( char * )"Cube" ) )
	{
		ProgramManagerObj->RemoveProgram(program);
	}
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
}

void ManyCubes::RenderCube()
{
    //Culling();
    glBindVertexArray(vao);
    glUniformMatrix4fv( mvp, 1, GL_FALSE,(float*)TransformObj->TransformGetModelViewProjectionMatrix() );
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
    glBindVertexArray(0);
}

void ManyCubes::Culling() {
    glEnable(GL_CULL_FACE);
    if (toogle) {
        LOGI("FRONT Face");
        glClearDepthf(0.0f);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_GREATER);
        glCullFace(GL_FRONT);
    }
    else {
        LOGI("BACK Face");
        glClearDepthf(1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_LESS);
        glCullFace(GL_BACK);
    }
}

void ManyCubes::InitModel()
{
	if (! ( program = ProgramManagerObj->Program( (char *)"Cube" ))){
		program = ProgramManagerObj->ProgramInit( (char *)"Cube" );
		ProgramManagerObj->AddProgram( program );
	}

	program->VertexShader	= ShaderManager::ShaderInit( VERTEX_SHADER_PRG,	GL_VERTEX_SHADER	);
	program->FragmentShader	= ShaderManager::ShaderInit( FRAGMENT_SHADER_PRG, GL_FRAGMENT_SHADER	);

    //////////////////////////////////////////////////////
    /////////// Vertex shader //////////////////////////
    //////////////////////////////////////////////////////
	CACHE *m = reserveCache( VERTEX_SHADER_PRG, true );

	if( m ) {
		if( !ShaderManager::ShaderCompile( program->VertexShader, ( char * )m->buffer, 1 ) ) exit( 1 );
        freeCache( m );
	}

	m = reserveCache( FRAGMENT_SHADER_PRG, true );
	if( m ) {
		if( !ShaderManager::ShaderCompile( program->FragmentShader, ( char * )m->buffer, 1 ) ) exit( 2 );
        freeCache( m );
	}

    if( !ProgramManagerObj->ProgramLink( program, 1 ) ) exit( 3 );

    glUseProgram( program->ProgramID );
    mvp    = ProgramManagerObj->ProgramGetUniformLocation( program, ( char* )"MODELVIEWPROJECTIONMATRIX" );
    attribVertex   = ProgramManagerObj->ProgramGetVertexAttribLocation(program, (char*)"VertexPosition");
    attribColor    = ProgramManagerObj->ProgramGetVertexAttribLocation(program, (char*)"VertexColor");
    int maxVertexAttribs = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
    LOGI("maxVertexAttribs :  %d attribVertex : %d, attribColor : %d", maxVertexAttribs, attribVertex, attribColor );

    InitBufferObject();
    return;
}

void ManyCubes::InitBufferObject() {
    unsigned short indexSize = sizeof( unsigned short ) * 36;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size + size, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, cubeVerts);
    glBufferSubData(GL_ARRAY_BUFFER, size, size, cubeColors);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexSize, cubeIndices);

    glEnableVertexAttribArray(attribVertex);
    glEnableVertexAttribArray(attribColor);
    glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribPointer(attribColor, 3, GL_FLOAT, GL_FALSE, 0, (void*)size);
    glBindVertexArray(0);
}

static float k = 0;

/*!
	Initialize the scene, reserve shaders, compile and cache program

	\param[in] None.
	\return None

*/
void ManyCubes::Render()
{
    glEnable( GL_DEPTH_TEST );
    glUseProgram( program->ProgramID );
    if(distance > 5)
        op = true;
    if(distance < 2.0)
        op = false;
    
    if (Animate){
        if(op)
            distance -= 0.1;
        else
            distance += 0.1;
    }

    // TransformObj->PrintMatrixMode();
    TransformObj->TransformRotate(k++, 1, 1, 1);
    // RenderPlus();
    //BehindCubes();
    RenderCube();
}


void ManyCubes::RenderCubeOfCubes()
{
	TransformObj->TransformTranslate(-distance * dimension/2,  -distance * dimension/2, -distance * dimension/2);
	for (int i = 0; i<dimension; i++){
		TransformObj->TransformTranslate(distance,  0.0, 0.0);
		TransformObj->TransformPushMatrix();
		for (int j = 0; j<dimension; j++){
			TransformObj->TransformTranslate(0.0,  distance, 0.0);
            TransformObj->TransformPushMatrix();
            for (int j = 0; j<dimension; j++){
                TransformObj->TransformTranslate(0.0,  0.0, distance);
                RenderCube();
            }
            TransformObj->TransformPopMatrix();
		}
		TransformObj->TransformPopMatrix();
	}
}

void ManyCubes::RenderPlus()
{
    RenderCube();
    TransformObj->TransformPushMatrix();
    for (int i=0; i<10; i++) {
        TransformObj->TransformTranslate(2,  0, 0.0);
        RenderCube();
    }
    TransformObj->TransformPopMatrix();
    TransformObj->TransformPushMatrix();
    for (int i=0; i<10; i++) {
        TransformObj->TransformTranslate(-2,  0, 0.0);
        RenderCube();
    }
    TransformObj->TransformPopMatrix();
    TransformObj->TransformPushMatrix();
    for (int i=0; i<10; i++) {
        TransformObj->TransformTranslate(0,  -2, 0);
        RenderCube();
    }
    TransformObj->TransformPopMatrix();
    TransformObj->TransformPushMatrix();
    for (int i=0; i<10; i++) {
        TransformObj->TransformTranslate(0,  2, 0);
        RenderCube();
    }
}

void ManyCubes::BehindCubes()
{
    RenderCube();
    TransformObj->TransformPushMatrix();
    TransformObj->TransformTranslate(-2, 0, -5);
    RenderCube();
    TransformObj->TransformTranslate(-2, 0, -5);
    RenderCube();
    TransformObj->TransformTranslate(-2, 0, -5);
    RenderCube();
    TransformObj->TransformPopMatrix();
}

void ManyCubes::TouchEventDown( float a, float b ) {
    toogle = !toogle;
}