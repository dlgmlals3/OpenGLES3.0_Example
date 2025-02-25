#include "InstanceCube.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "Cache.h"
//#include "Transform.h"
#include "constant.h"

#define VERTEX_SHADER_PRG			( char * )"shader/GeometricInstancingVertex.glsl"
#define FRAGMENT_SHADER_PRG			( char * )"shader/GeometricInstancingFragment.glsl"

#define VERTEX_LOCATION 0
#define COLOR_LOCATION 1
#define MATRIX1_LOCATION 2
#define MATRIX2_LOCATION 3
#define MATRIX3_LOCATION 4
#define MATRIX4_LOCATION 5

// Global Object Declaration

/*!
	Simple Default Constructor

	\param[in] None.
	\return None

*/
InstanceCube::InstanceCube( Renderer* parent )
{
	if (!parent)
		return;

	MapRenderHandler	= parent;
	ProgramManagerObj	= parent->RendererProgramManager();
	TransformObj		= parent->RendererTransform();
	modelType 			= CubeType;
}


/*!
	Simple Destructor

	\param[in] None.
	\return None

*/
InstanceCube::~InstanceCube()
{
	PROGRAM* program = NULL;
	if ( program = ProgramManagerObj->Program( ( char * )"Cube" ) )
	{
		ProgramManagerObj->RemoveProgram(program);
	}
	glDeleteBuffers(1, &vId);
	glDeleteBuffers(1, &iId);
}

/*!
	Initialize the scene, reserve shaders, compile and cache program

	\param[in] None.
	\return None

*/
void InstanceCube::InitModel()
{
	if (! ( program = ProgramManagerObj->Program( (char *)"Cube" ))){
		program = ProgramManagerObj->ProgramInit( (char *)"Cube" );
		ProgramManagerObj->AddProgram( program );
	}

    //Initialize Vertex and Fragment shader
	program->VertexShader	= ShaderManager::ShaderInit( VERTEX_SHADER_PRG,	GL_VERTEX_SHADER	);
	program->FragmentShader	= ShaderManager::ShaderInit( FRAGMENT_SHADER_PRG, GL_FRAGMENT_SHADER	);

    // Compile Vertex shader
	CACHE *m = reserveCache( VERTEX_SHADER_PRG, true );
	if( m ) {
		if( !ShaderManager::ShaderCompile( program->VertexShader, ( char * )m->buffer, 1 ) ) exit( 1 );
        freeCache( m );
	}

    // Compile Fragment shader
	m = reserveCache( FRAGMENT_SHADER_PRG, true );
	if( m ) {
		if( !ShaderManager::ShaderCompile( program->FragmentShader, ( char * )m->buffer, 1 ) ) exit( 2 );
        freeCache( m );
	}

    // Link program
    if( !ProgramManagerObj->ProgramLink( program, 1 ) ) exit( 3 );

    glUseProgram( program->ProgramID );

    // Create VBO
 	size = 24*sizeof(float);
    glGenBuffers(1, &vId);
	glBindBuffer( GL_ARRAY_BUFFER, vId );
	glBufferData( GL_ARRAY_BUFFER, size + size, 0, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0,			size,	cubeVerts );
	glBufferSubData( GL_ARRAY_BUFFER, size,			size,	cubeColors );

    // Create VBO for transformation matrix
    glGenBuffers(1, &matrixId);
	glBindBuffer( GL_ARRAY_BUFFER, matrixId );


    glm::mat4 transformMatrix[dimension][dimension][dimension];
    glBufferData(GL_ARRAY_BUFFER, sizeof(transformMatrix) , 0, GL_DYNAMIC_DRAW);

    // Create IBO
	unsigned short indexSize = sizeof( unsigned short )*36;
    glGenBuffers(1, &iId);
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, iId );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, indexSize, 0, GL_STATIC_DRAW );
	glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, indexSize,	cubeIndices );

    glGenVertexArrays(1, &Vertex_VAO_Id);
    glBindVertexArray(Vertex_VAO_Id);

    // Create VBO  and set attribute parameters
	glBindBuffer( GL_ARRAY_BUFFER, vId );
    glEnableVertexAttribArray(VERTEX_LOCATION);
    glEnableVertexAttribArray(COLOR_LOCATION);
    glVertexAttribPointer(VERTEX_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribPointer(COLOR_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, (void*)size);

    // Create VBO for transformation matrix and set attribute parameters
	glBindBuffer( GL_ARRAY_BUFFER, matrixId );
    glEnableVertexAttribArray(MATRIX1_LOCATION);
    glEnableVertexAttribArray(MATRIX2_LOCATION);
    glEnableVertexAttribArray(MATRIX3_LOCATION);
    glEnableVertexAttribArray(MATRIX4_LOCATION);

    glVertexAttribPointer(MATRIX1_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(float) * 0));
    glVertexAttribPointer(MATRIX2_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(float) * 4));
    glVertexAttribPointer(MATRIX3_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(float) * 8));
    glVertexAttribPointer(MATRIX4_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(float) * 12));

    glVertexAttribDivisor(MATRIX1_LOCATION, 1);
    glVertexAttribDivisor(MATRIX2_LOCATION, 1);
    glVertexAttribDivisor(MATRIX3_LOCATION, 1);
    glVertexAttribDivisor(MATRIX4_LOCATION, 1);

    // Bind IBO
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, iId );

    // Make sure the VAO is not changed from outside code
    glBindVertexArray(0);
}

/*!
	Initialize the scene, reserve shaders, compile and cache program

	\param[in] None.
	\return None

*/
void InstanceCube::Render()
{
    glEnable( GL_DEPTH_TEST );

    glUseProgram( program->ProgramID );
    RenderCube();
}

void InstanceCube::RenderCube()
{
    // camera seting
    TransformObj->TransformSetMatrixMode(VIEW_MATRIX);
    TransformObj->TransformTranslate(0, 0, -100);


    glBindBuffer( GL_ARRAY_BUFFER, matrixId );
    GLsizeiptr length = (long)sizeof(glm::mat4) * dimension * dimension * dimension;
    glm::mat4* matrixBuf = (glm::mat4*)glMapBufferRange( GL_ARRAY_BUFFER, 0,
                                                         length, GL_MAP_WRITE_BIT);

    static float l = 0;
    float dist = -distance * dimension / 2.0;
    TransformObj->TransformSetMatrixMode(MODEL_MATRIX);
    TransformObj->TransformTranslate(dist,  dist, dist);
    TransformObj->TransformRotate(l++, 0, 1, 0);


    glm::mat4 projectionMatrix  = *TransformObj->TransformGetProjectionMatrix();
    glm::mat4 modelMatrix       = *TransformObj->TransformGetModelMatrix();
    glm::mat4 viewMatrix        = *TransformObj->TransformGetViewMatrix();
    int instance = 0;

    for ( int i = 0; i < dimension; i++ )
    {
        for ( int j = 0; j < dimension; j++ )
        {
            for ( int k = 0; k < dimension; k++ )
            {
                /*
                glm::vec3 offset = glm::vec3( (float)i * distance , (float)j * distance, (float)k * distance);
                glm::mat4x4 transform = glm::mat4(1.0f);
                transform = glm::translate(transform, offset);
                //transform = glm::rotate( transform, l, glm::vec3(1.0, 0.0, 0.0));
                matrixBuf[instance++] = projectionMatrix * viewMatrix * transform;
                 */
                matrixBuf[instance++] = projectionMatrix * viewMatrix *
                        glm::translate(modelMatrix,
                               glm::vec3( i*distance , j*distance, k*distance)) * glm::rotate( modelMatrix, l, glm::vec3(1.0, 0.0, 0.0));
            }
        }
    }
    glUnmapBuffer ( GL_ARRAY_BUFFER );

    glBindVertexArray(Vertex_VAO_Id);
    glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0, dimension * dimension * dimension);
    //glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0, 5);
}

/*!
 This function handle Touch event down action.

 \param[in] x and y screen pixel position.

 \return None.
 */
void InstanceCube::TouchEventDown( float x, float y )
{
	Animate = !Animate;
}

