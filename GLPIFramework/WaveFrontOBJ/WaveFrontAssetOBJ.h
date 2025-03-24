#ifndef WAVEFRONTASSETOBJ_H
#define WAVEFRONTASSETOBJ_H

#include <vector>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "GLutils.h"

#define MAX_FILE_NAME 1024

class Vertex
{
public:
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec4 tangent;

    Vertex() : position(0.0f), uv(0.0f), normal(0.0f), tangent(0.0f) {}
};

class FaceIndex
{
public:
    short vertexIndex;
    short normalIndex;
    short uvIndex;

    FaceIndex(short v, short u, short n) : vertexIndex(v), uvIndex(u), normalIndex(n) {}
};

struct Mesh
{
    char fileName[MAX_FILE_NAME];

    std::vector<FaceIndex> vecFaceIndex;
    std::vector<Vertex> vertices;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec4> tangents;
    std::vector<unsigned short> indices;

    Mesh() { memset(fileName, 0, MAX_FILE_NAME); }
};

class OBJMesh
{
    Mesh objMeshModel;
    int IndexCount;
    int bufferPos = 0;             // 현재 버퍼 내 위치
    int bufferSize = 0;            // 현재 버퍼에 읽힌 데이터 크기
    char assetBuffer[1024];
public:
    OBJMesh(){};
    ~OBJMesh(){};

    Mesh* ParseObjModel(AAssetManager* assetManager, const char* path, bool flatShading = false);
    int IndexTotal() { return IndexCount; }
    void PrintModelInfo();
    void PrintParseData();

private:
    bool ParseFileInfo(AAssetManager* assetManager, const char* path);
    bool ScanVertexNormalAndUV(char *buffer);
    bool ScanFaceIndex(char *buffer);
    bool CalculateNormal(bool flatShading);
    bool CalculateTangents();
    void CreateInterleavedArray();
    bool ClearMesh();
};

#endif // WAVEFRONTOBJ_H
