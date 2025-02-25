#include "InstanceCube.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "Cache.h"
//#include "Transform.h"
#include "constant.h"
#include "type_ptr.hpp"
#include "string_cast.hpp"

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
    // ============================
    // 📌 1. 셰이더 프로그램 초기화
    // ============================
    if (!(program = ProgramManagerObj->Program((char *)"Cube"))) {
        program = ProgramManagerObj->ProgramInit((char *)"Cube");
        ProgramManagerObj->AddProgram(program);
    }

    // ============================
    // 📌 2. 셰이더 로드 및 컴파일
    // ============================
    program->VertexShader   = ShaderManager::ShaderInit(VERTEX_SHADER_PRG, GL_VERTEX_SHADER);
    program->FragmentShader = ShaderManager::ShaderInit(FRAGMENT_SHADER_PRG, GL_FRAGMENT_SHADER);

    // 📌 2-1. 정점 셰이더(Shader) 컴파일
    CACHE *m = reserveCache(VERTEX_SHADER_PRG, true);
    if (m) {
        if (!ShaderManager::ShaderCompile(program->VertexShader, (char *)m->buffer, 1))
            exit(1);
        freeCache(m);
    }

    // 📌 2-2. 프래그먼트 셰이더(Shader) 컴파일
    m = reserveCache(FRAGMENT_SHADER_PRG, true);
    if (m) {
        if (!ShaderManager::ShaderCompile(program->FragmentShader, (char *)m->buffer, 1))
            exit(2);
        freeCache(m);
    }

    // 📌 2-3. 셰이더 프로그램 링크
    if (!ProgramManagerObj->ProgramLink(program, 1))
        exit(3);

    glUseProgram(program->ProgramID);  // 프로그램 활성화


    // ============================
    // 📌 3. VBO (Vertex Buffer Object) 생성 및 데이터 전송
    // ============================
    size = 24 * sizeof(float);
    glGenBuffers(1, &vId);
    glBindBuffer(GL_ARRAY_BUFFER, vId);
    glBufferData(GL_ARRAY_BUFFER, size + size, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, cubeVerts);   // 정점 데이터 업로드
    glBufferSubData(GL_ARRAY_BUFFER, size, size, cubeColors); // 색상 데이터 업로드

    // ============================
    // 📌 4. 변환 행렬용 VBO 생성
    // ============================
    glGenBuffers(1, &matrixId);
    glBindBuffer(GL_ARRAY_BUFFER, matrixId);

    glm::mat4 transformMatrix[dimension][dimension][dimension]; // 3D 행렬 배열 생성
    glBufferData(GL_ARRAY_BUFFER, sizeof(transformMatrix), 0, GL_DYNAMIC_DRAW); // 동적 할당

    // ============================
    // 📌 5. IBO (Index Buffer Object) 생성
    // ============================
    unsigned short indexSize = sizeof(unsigned short) * 36;
    glGenBuffers(1, &iId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexSize, cubeIndices); // 인덱스 데이터 업로드

    // ============================
    // 📌 6. VAO (Vertex Array Object) 생성 및 설정
    // ============================
    glGenVertexArrays(1, &Vertex_VAO_Id);
    glBindVertexArray(Vertex_VAO_Id);

    // 📌 6-1. VBO 바인딩 및 속성 설정
    glBindBuffer(GL_ARRAY_BUFFER, vId);
    glEnableVertexAttribArray(VERTEX_LOCATION);
    glEnableVertexAttribArray(COLOR_LOCATION);
    glVertexAttribPointer(VERTEX_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribPointer(COLOR_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, (void*)size);

    // 📌 6-2. 변환 행렬용 VBO 속성 설정
    glBindBuffer(GL_ARRAY_BUFFER, matrixId);
    glEnableVertexAttribArray(MATRIX1_LOCATION);
    glEnableVertexAttribArray(MATRIX2_LOCATION);
    glEnableVertexAttribArray(MATRIX3_LOCATION);
    glEnableVertexAttribArray(MATRIX4_LOCATION);

    // 한번에 mat4 타입을 전달할 수 없기에, float 4개씩 묶음,
    // 현재 바인딩 된 VBO특정 위치에서 데이터를 읽어와서 어트리뷰트로 사용하도록 지정.
    glVertexAttribPointer(MATRIX1_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(float) * 0)); // 1 행
    glVertexAttribPointer(MATRIX2_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(float) * 4)); // 2 행
    glVertexAttribPointer(MATRIX3_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(float) * 8));
    glVertexAttribPointer(MATRIX4_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(float) * 12));

    // 📌 6-3. 인스턴스별 변환 행렬 적용 (Divisor 설정)
    // 0 : 각 정점 마다  vertex 어트리뷰트 데이터 사용
    // 1 : 각 인스턴스 마다 vertex 어트리뷰트 데이터 사용
    // n : n개의 인스턴스 마다 vertex 어트리뷰트 데이터 사용
    glVertexAttribDivisor(MATRIX1_LOCATION, 1);
    glVertexAttribDivisor(MATRIX2_LOCATION, 1);
    glVertexAttribDivisor(MATRIX3_LOCATION, 1);
    glVertexAttribDivisor(MATRIX4_LOCATION, 1);

    // 📌 6-4. IBO 바인딩
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iId);

    // 📌 6-5. VAO를 보호 (외부 코드가 변경하지 못하도록)
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

    RenderOrigin();
    //RenderCube();
    //RenderRandomCube();
}

// 인스턴스 렌더링 사용하지 않고
void InstanceCube::RenderOrigin() {
    char loc = ProgramManagerObj->ProgramGetUniformLocation(program, (char*)"MODELVIEWPROJECTIONMATRIX");
    float dist = -distance * dimension / 2.0;
    static float l = 0;

    TransformObj->TransformSetMatrixMode(VIEW_MATRIX);
    TransformObj->TransformTranslate(0, 0, -150);

    TransformObj->TransformSetMatrixMode(MODEL_MATRIX);
    TransformObj->TransformRotate(l++, 0, 1, 0);
    TransformObj->TransformTranslate(dist,  dist, dist);

    glm::mat4 projectionMatrix  = *TransformObj->TransformGetProjectionMatrix();
    glm::mat4 viewMatrix        = *TransformObj->TransformGetViewMatrix();
    glm::mat4 modelMatrix       = *TransformObj->TransformGetModelMatrix();

    glBindVertexArray(Vertex_VAO_Id);
    //LOGI("dlgmlasl3 loc : %d vao : %d", loc, Vertex_VAO_Id);

    for ( int i = 0; i < dimension; i++ ) {
        for (int j = 0; j < dimension; j++) {
            for (int k = 0; k < dimension; k++) {
                glm::mat4 mvp = projectionMatrix * viewMatrix
                        * glm::translate(modelMatrix, glm::vec3( i * 5, j * 5, k * 5));
                glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mvp));
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
                CheckGLAPI("1");
            }
        }
    }
}

void InstanceCube::RenderRandomCube() {
    static float l = 0;
    float dist = -distance * dimension / 2.0;

    TransformObj->TransformSetMatrixMode(VIEW_MATRIX);
    TransformObj->TransformTranslate(0, 0, -150);
    glBindBuffer(GL_ARRAY_BUFFER, matrixId);

    TransformObj->TransformSetMatrixMode(MODEL_MATRIX);
    TransformObj->TransformRotate(l++, 0, 1, 0);
    TransformObj->TransformTranslate(dist,  dist, dist);

    glm::mat4 projectionMatrix = *(TransformObj->TransformGetProjectionMatrix());
    glm::mat4 viewMatrix = *(TransformObj->TransformGetViewMatrix());
    glm::mat4 modelMatrix = *(TransformObj->TransformGetModelMatrix());

    int count = dimension * dimension * dimension;
    //count = 10;
    GLsizeiptr length = (long)sizeof(glm::mat4) * count;

    auto matrixBuf = (glm::mat4*) glMapBufferRange(GL_ARRAY_BUFFER, 0,
                                                         length, GL_MAP_WRITE_BIT);
    int instance = 0;
    for (int i=0; i<count; i++) {
        int x = 3153 * i % 80;
        int y = 2421 * i % 80;
        int z = 1311 * i % 80;

        matrixBuf[instance++] = projectionMatrix * viewMatrix
                                * glm::translate(modelMatrix, glm::vec3(x, y, z));
    }
    glUnmapBuffer ( GL_ARRAY_BUFFER );
    glBindVertexArray(Vertex_VAO_Id);

    GLint maxIndex;
    glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &maxIndex);
    // GPU 한번 드로우 콜에서 사용할 수 있는 최대 인덱스 개수 1048576 --> glDrawElementsInstanced count 넘으면 안됌.

    // unsigned short indexSize = sizeof(unsigned short) * 36; <-- 으로 인하여 타입 UNSIGNED_SHORT
    glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT,
                            (void*)0, dimension * dimension * dimension);
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
    TransformObj->TransformRotate(l++, 0, 1, 0);
    TransformObj->TransformTranslate(dist,  dist, dist);

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
                matrixBuf[instance++] = projectionMatrix * viewMatrix *
                        glm::translate(modelMatrix, glm::vec3( i*distance , j*distance, k*distance));
            }
        }
    }
    glUnmapBuffer ( GL_ARRAY_BUFFER );

    glBindVertexArray(Vertex_VAO_Id);
    glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0, dimension * dimension * dimension);
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

