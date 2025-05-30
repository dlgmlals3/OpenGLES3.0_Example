#include "ObjLoader.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "Cache.h"
#include "constant.h"
//#import <fstream>
using namespace glm;

//#define VERTEX_SHADER_PRG			( char * )"shader/GouraudShadeVertex.glsl"
//#define FRAGMENT_SHADER_PRG			( char * )"shader/GouraudShadeFragment.glsl"

// #define VERTEX_SHADER_PRG			( char * )"shader/PhongShadeVertex.glsl"
// #define FRAGMENT_SHADER_PRG		( char * )"shader/PhongShadeFragment.glsl"

#define VERTEX_SHADER_PRG			( char * )"shader/MultipleLightVertex.glsl"
#define FRAGMENT_SHADER_PRG		( char * )"shader/MultipleLightFragment.glsl"

#define VERTEX_POSITION 0
#define NORMAL_POSITION 1
#define TEX_COORD 2

// Namespace used
using std::ifstream;
using std::ostringstream;

// Model Name Array
#define STRING_LENGTH 100
char ModelNames[][STRING_LENGTH]= {"Cube.obj","Cylinder.obj", "Torus.obj", "Monkey.obj", "IsoSphere.obj"};
/*!
	Simple Default Constructor

	\param[in] None.
	\return None

*/
enum TaskType {
    SWITCH_MESH
};

struct RenderTask {
    TaskType type;
};

std::mutex taskQueueMutex;
std::queue<RenderTask> renderTaskQueue;

ObjLoader::ObjLoader( Renderer* parent )
{
	if (!parent)
		return;

	MapRenderHandler	= parent;
	ProgramManagerObj	= parent->RendererProgramManager();
	TransformObj		= parent->RendererTransform();
	modelType 			= ObjFileType;
    glEnable	( GL_DEPTH_TEST );

    ModelNumber = 1;
    LoadMesh();
}

void ObjLoader::ReleaseMeshResources()
{
    LOGI("ReleaseMeshResources");
    if (OBJ_VAO_Id){
        glDeleteVertexArrays(1, &OBJ_VAO_Id);
        OBJ_VAO_Id = 0;
    }
    if (vertexBuffer) {
        glDeleteBuffers(1, &vertexBuffer);
        vertexBuffer = 0;
    }
}

void ObjLoader::SwitchMesh()
{
    LOGI("SwitchMesh");
    // Release the old resources
    ReleaseMeshResources();
    
    // Get the new mesh model number
    ModelNumber++;
    ModelNumber %= sizeof(ModelNames) / (sizeof(char)*STRING_LENGTH);
    
    // Load the new mesh model
    LoadMesh();
}

void ObjLoader::LoadMesh()
{
    LOGI("LoadMesh 실행 스레드 : %ld", std::this_thread::get_id());

    char fname[MAX_PATH]= {""};
    strcpy( fname, "Models/" );

    strcat( fname, ModelNames[ModelNumber]);

    LOGI("fname : %s", fname);
    bool flatNormal = false;

    objMeshModel    = waveFrontObjectModel.ParseObjModel(MapRenderHandler->getAssetManager(), fname, flatNormal);
    IndexCount      = waveFrontObjectModel.IndexTotal();

    stride          = (2 * sizeof(vec3) )+ sizeof(vec2) + sizeof(vec4);
    offset          = ( GLvoid*) ( sizeof(glm::vec3) + sizeof(vec2) );
    offsetTexCoord  = ( GLvoid*) ( sizeof(glm::vec3) );

    // Create the VBO for our obj model vertices.
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, objMeshModel->vertices.size() * sizeof(objMeshModel->vertices[0]), &objMeshModel->vertices[0], GL_STATIC_DRAW);

    // Create the VAO, this will store the vertex attributes into vectored state.
    glGenVertexArrays(1, &OBJ_VAO_Id);
    glBindVertexArray(OBJ_VAO_Id);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glEnableVertexAttribArray(VERTEX_POSITION);
    glEnableVertexAttribArray(TEX_COORD);
    glEnableVertexAttribArray(NORMAL_POSITION);
    glVertexAttribPointer(VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glVertexAttribPointer(TEX_COORD, 2, GL_FLOAT, GL_FALSE, stride, offsetTexCoord);
    glVertexAttribPointer(NORMAL_POSITION, 3, GL_FLOAT, GL_FALSE, stride, offset);
    glBindVertexArray(0);
    
    objMeshModel->vertices.clear();
}
/*!
	Simple Destructor

	\param[in] None.
	\return None

*/
ObjLoader::~ObjLoader()
{
	PROGRAM* program = NULL;
	if ( program = ProgramManagerObj->Program( ( char * ) "Shader" ) )
	{
		ProgramManagerObj->RemoveProgram(program);
	}
}

/*!
	Initialize the scene, reserve shaders, compile and cache program

	\param[in] None.
	\return None

*/
void ObjLoader::InitModel()
{
	ProgramManager* ProgramManagerObj   = MapRenderHandler->RendererProgramManager();
	Transform*	TransformObj            = MapRenderHandler->RendererTransform();


	if (! ( program = ProgramManagerObj->Program( ( char * )"Shader" ) ) )
	{
		program = ProgramManagerObj->ProgramInit( ( char * )"Shader" );
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
    
    // Get Material property uniform variables
    char MaterialAmbient  = ProgramManagerObj->ProgramGetUniformLocation(program,(char*)"MaterialAmbient");
    char MaterialSpecular = ProgramManagerObj->ProgramGetUniformLocation(program,(char*)"MaterialSpecular");
    char MaterialDiffuse  = ProgramManagerObj->ProgramGetUniformLocation(program,(char*)"MaterialDiffuse");
    char ShininessFactor  = ProgramManagerObj->ProgramGetUniformLocation(program,(char*)"ShininessFactor");
    
    // Get Light property uniform variables
    char LightAmbient     = ProgramManagerObj->ProgramGetUniformLocation(program,(char*)"LightAmbient");
    char LightSpecular    = ProgramManagerObj->ProgramGetUniformLocation(program,(char*)"LightSpecular");
    char LightDiffuse     = ProgramManagerObj->ProgramGetUniformLocation(program,(char*)"LightDiffuse");
    char LightPosition    = ProgramManagerObj->ProgramGetUniformLocation(program,(char*)"LightPosView");
    
    if ( MaterialAmbient >= 0 ) {
        LOGI("MaterialAmbient Setting");
        glUniform3f(MaterialAmbient, 0.1f, 0.1f, 0.1f);
    }
    
    if ( MaterialSpecular >= 0) {
        LOGI("MaterialSpecular Setting");
        glUniform3f( MaterialSpecular, 1.0, 0.5, 0.5 );
    }
    
    if ( MaterialDiffuse >= 0 ) {
        LOGI("MaterialDiffuse Setting");
        glm::vec3 color = glm::vec3(0.75, 0.375, 0.0);
        glUniform3f( MaterialDiffuse, color.r, color.g, color.b );
    }

    if ( LightAmbient >= 0 ) {
        LOGI("LightAmbient Setting");
        glUniform3f( LightAmbient, 1.0f, 1.0f, 1.0f );
    }
    
    if ( LightSpecular >=  0 ) {
        LOGI("LightSpecular Setting");
        glUniform3f( LightSpecular, 1.0, 1.0, 1.0 );
    }

    if ( LightDiffuse >= 0 ){
        LOGI("LightDiffuse Setting");
        glUniform3f(LightDiffuse, 1.0f, 1.0f, 1.0f);
    }

    if ( ShininessFactor >= 0 ){
        LOGI("ShininessFactor Setting");
        glUniform1f(ShininessFactor, 40);
    }

    if ( LightPosition >= 0 ){
        LOGI("LightPosition Setting");
        glm::vec3 lightPosition(0, 10, 0);
        glm::vec4 lightView = (*TransformObj->TransformGetModelViewMatrix()) * glm::vec4(lightPosition, 1);
        glm::vec3 lightViewVec3 = vec3(lightView.x, lightView.y, lightView.z);
        glUniform3fv(LightPosition, 1, (float*)&lightViewVec3);
    }

    InitMultiLight();
    
    MVP = ProgramManagerObj->ProgramGetUniformLocation( program, ( char* )"ModelViewProjectionMatrix" );
    MV  = ProgramManagerObj->ProgramGetUniformLocation( program, ( char* )"ModelViewMatrix" );
    NormalMatrix  = ProgramManagerObj->ProgramGetUniformLocation(program, (char*)"NormalMatrix");
    return;
}

void ObjLoader::InitMultiLight() {
    char LightPositionArray   = ProgramManagerObj->ProgramGetUniformLocation(program,(char*)"LightPositionArray[0]");
    char LightDiffuseArray = ProgramManagerObj->ProgramGetUniformLocation(program,(char*)"LightDiffuseArray[0]");

    float lightpositions[12] = {
            -30.0, 0.0, 5.0,
            0.0, 10.0, 5.0,
            10.0, 0.0, 5.0,
            0.0, -10.0, 5.0
    };

    float lightdiffusecolors[12] = {
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0
    };

    if (LightPositionArray >= 0){
        glUniform3fv(LightPositionArray, sizeof(lightpositions) / sizeof(float), lightpositions);
    }


    if (LightDiffuseArray >= 0){
        glUniform3fv(LightDiffuseArray, sizeof(lightdiffusecolors) / sizeof(float), lightdiffusecolors);
    }
}
/*!
	Initialize the scene, reserve shaders, compile and cache program

	\param[in] None.
	\return None

*/
void ObjLoader::Render()
{
    taskQueueMutex.lock();
    while (!renderTaskQueue.empty()) {
        RenderTask task = renderTaskQueue.front();
        renderTaskQueue.pop();
        taskQueueMutex.unlock();

        switch (task.type) {
            case SWITCH_MESH:
                SwitchMesh();
                break;
            default:
                break;
        }
        taskQueueMutex.lock();
    }
    taskQueueMutex.unlock();

    // Use Ambient program
    glUseProgram(program->ProgramID);

    // Apply Transformation.
	TransformObj->TransformPushMatrix();
	TransformObj->TransformTranslate(0, 0, 0);
	TransformObj->TransformScale(1.0,1.0,1.0);
    static float rot = 0.0;
	TransformObj->TransformRotate(rot++ , 0.0, 1.0, 0.0);
    glUniformMatrix4fv( MVP, 1, GL_FALSE,( float * )TransformObj->TransformGetModelViewProjectionMatrix() );
    glUniformMatrix4fv( MV, 1, GL_FALSE,( float * )TransformObj->TransformGetModelViewMatrix() );
    glm::mat4 matrix    = *(TransformObj->TransformGetModelViewMatrix());
    glm::mat3 normalMat = glm::mat3( glm::vec3(matrix[0]), glm::vec3(matrix[1]), glm::vec3(matrix[2]) );
    glUniformMatrix3fv( NormalMatrix, 1, GL_FALSE, (float*)&normalMat );
    TransformObj->TransformPopMatrix();

    // Bind with Vertex Array Object for OBJ
    glBindVertexArray(OBJ_VAO_Id);
    
    // Draw Geometry
    glDrawArrays(GL_TRIANGLES, 0, IndexCount );
    glBindVertexArray(0);
}

void ObjLoader::TouchEventDown( float x, float y )
{
    if (x > 900 && y > 2000) {
        std::lock_guard<std::mutex> lock(taskQueueMutex);
        renderTaskQueue.push({ SWITCH_MESH });
    }
    else {
        RenderPrimitive++;
        if (RenderPrimitive > 2) {
            RenderPrimitive = 0;
        }
    }
}

void ObjLoader::TouchEventDoubleClick( float x, float y )
{
    LOGI("TouchEventDoubleClick");
}

void ObjLoader::TouchEventMove( float x, float y )
{
    LOGI("TouchEventMove");
}

void ObjLoader::TouchEventRelease( float x, float y )
{
}
