#include "ObjLoader.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "Cache.h"
#include "constant.h"
//#import <fstream>
using namespace glm;

#define VERTEX_SHADER_PRG			( char * )"shader/VertexObj.glsl"
#define FRAGMENT_SHADER_PRG			( char * )"shader/FragmentObj.glsl"

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
    ModelNumber %= sizeof(ModelNames)/(sizeof(char)*STRING_LENGTH);
    
    // Load the new mesh model
    LoadMesh();
}

void ObjLoader::LoadMesh()
{
    LOGI("LoadMesh 실행 스레드 : %ld", std::this_thread::get_id());

    char fname[MAX_PATH]= {""};
    strcpy( fname, "Models/" );

    strcat( fname, ModelNames[ModelNumber]);


    objMeshModel    = waveFrontObjectModel.ParseObjModel(MapRenderHandler->getAssetManager(), fname);
    IndexCount      = waveFrontObjectModel.IndexTotal();

    stride          = (2 * sizeof(vec3) )+ sizeof(vec2) + sizeof(vec4);
    offset          = ( GLvoid*) ( sizeof(glm::vec3) + sizeof(vec2) );
    offsetTexCoord  = ( GLvoid*) ( sizeof(glm::vec3) );

    // Create the VAO, this will store the vertex attributes into vectore state.
    glGenVertexArrays(1, &OBJ_VAO_Id);
    glBindVertexArray(OBJ_VAO_Id);

    // Create the VBO for our obj model vertices.
    glGenBuffers(1, &vertexBuffer);
    debugCode();

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    LOGI("Load Mesh : %s, vertex size : %d", fname, objMeshModel->vertices.size());

    int size = objMeshModel->vertices.size() * sizeof(objMeshModel->vertices[0]);
    glBufferData(GL_ARRAY_BUFFER, size, &objMeshModel->vertices[0], GL_STATIC_DRAW);
    

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    LOGI("objID : %d, vertexBuffer : %d", OBJ_VAO_Id, vertexBuffer);
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
	if ( program = ProgramManagerObj->Program( ( char * )"ShaderObj" ) )
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


	if (! ( program = ProgramManagerObj->Program( ( char * )"ShaderObj" ) ) )
	{
		program = ProgramManagerObj->ProgramInit( ( char * )"ShaderObj" );
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

    char MaterialDiffuse  = ProgramManagerObj->ProgramGetUniformLocation(program, (char*)"MaterialDiffuse");
    glm::vec3 color = glm::vec3(1.0, 0.5, 0.0) * 0.75f;
    if (MaterialDiffuse >= 0){
        glUniform3f(MaterialDiffuse, color.r, color.g, color.b);
    }

    MVP     = ProgramManagerObj->ProgramGetUniformLocation( program, ( char* )"MODELVIEWPROJECTIONMATRIX" );
    return;
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
	TransformObj->TransformTranslate(0,0,-3);
	TransformObj->TransformScale(1.0,1.0,1.0);
    static float rot = 0.0;
	TransformObj->TransformRotate(rot++ , 1.0, 1.0, 1.0);
    glUniformMatrix4fv( MVP, 1, GL_FALSE,( float * )TransformObj->TransformGetModelViewProjectionMatrix() );
	TransformObj->TransformPopMatrix();

    // Bind with Vertex Array Object for OBJ
    glBindVertexArray(OBJ_VAO_Id);
    
    // Draw Geometry
    if ( RenderPrimitive == 0 ){
        glDrawArrays(GL_POINTS, 0, IndexCount );
    }
    else if ( RenderPrimitive == 1 ){
        glDrawArrays(GL_LINES, 0, IndexCount );
    }
    else {
        glDrawArrays(GL_TRIANGLES, 0, IndexCount );
    }

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
