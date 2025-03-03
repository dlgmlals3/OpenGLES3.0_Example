#include "WaveFrontAssetOBJ.h"
#include <cstring>
#include <iostream>
#include <cstdio>

using namespace std;
using namespace glm;

bool OBJMesh::ParseFileInfo(AAssetManager* assetManager, const char* path)
{
    AAsset* asset = AAssetManager_open(assetManager, path, AASSET_MODE_STREAMING);
    if (!asset) {
        printf("Failed to open asset: %s\n", path);
        return false;
    }

    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE + 1]; // 널 문자 고려
    std::vector<std::string> sentence; // 한 줄씩 저장할 벡터
    std::string remaining = "";   // 읽다 만 줄 저장
    int readBytes;

    while ((readBytes = AAsset_read(asset, buffer, BUFFER_SIZE)) > 0) {
        buffer[readBytes] = '\0'; // Null-terminate
        std::string data = remaining + buffer;  // 이전 읽다 만 줄과 합치기
        remaining.clear(); // 남은 데이터 초기화

        size_t pos = 0;
        size_t newlinePos;

        // **줄바꿈(`\n`)이 있는지 검사**
        while ((newlinePos = data.find('\n', pos)) != std::string::npos) {
            std::string line = data.substr(pos, newlinePos - pos); // 한 줄 가져오기
            pos = newlinePos + 1;

            if (!line.empty()) {
                sentence.push_back(line); // 한 줄씩 vector에 추가
            }
        }

        // **끊긴 줄이 있으면 저장 (남은 부분)**
        if (pos < data.size()) {
            remaining = data.substr(pos);
        }
    }

    // **파일 끝에서 남은 줄 처리**
    if (!remaining.empty()) {
        sentence.push_back(remaining);
    }

    AAsset_close(asset);

    // **디버깅: 저장된 모든 줄 출력**
    int vertexCnt = 0;
    int faceCnt = 0;
    for (const auto& s : sentence) {
        switch (s[0]) {
            case '#':
            case 'u':
            case 's':
            case 'g':
                break;
            case 'v':  // Vertex data
                vertexCnt++;
                ScanVertexNormalAndUV(const_cast<char *>(s.c_str()));
                break;
            case 'f':  // Face data
                faceCnt++;
                ScanFaceIndex(const_cast<char *>(s.c_str()));
                break;
        }
    }

    return true;
}


bool OBJMesh::ScanVertexNormalAndUV(char *buffer)
{
    float x, y, z, u, v;
    char c = buffer[1];
    switch (c) {
        case ' ':
            if (sscanf(&buffer[1], "%f %f %f", &x, &y, &z) == 3) {
                objMeshModel.positions.push_back(vec3(x, y, z));
            }
            break;
        case 'n':
            if (sscanf(&buffer[1], "%f %f %f", &x, &y, &z) == 3) {
                objMeshModel.normals.push_back(vec3(x, y, z));
            }
            break;
        case 't':
            if (sscanf(&buffer[1], "%f %f", &u, &v) == 2) {
                objMeshModel.uvs.push_back(vec2(u, v));
            }
            break;
        default:
            return false;
    }
    return true;
}
#include <vector>
#include <cstring>
#include <cstdlib>

bool OBJMesh::ScanFaceIndex(char *buffer)
{
    std::vector<int> vertexIndices, uvIndices, normalIndices;

    // "f"를 건너뛰고 나머지 부분을 처리
    char* token = strtok(buffer + 1, " ");

    while (token)
    {
        int vIndex = -1, uvIndex = -1, nIndex = -1;

        // 정점/UV/노멀이 모두 있는 경우 (v/vt/vn)
        if (strstr(token, "//")) // v//vn (UV 없음)
        {
            sscanf(token, "%d//%d", &vIndex, &nIndex);
        }
        else if (strchr(token, '/')) // v/vt 또는 v/vt/vn
        {
            int matchCount = sscanf(token, "%d/%d/%d", &vIndex, &uvIndex, &nIndex);
            if (matchCount == 2) // v/vt (노멀 없음)
            {
                nIndex = -1;
            }
        }
        else // 정점만 있는 경우 (v)
        {
            sscanf(token, "%d", &vIndex);
        }

        // 인덱스 저장 (OBJ 인덱스는 1부터 시작하므로 -1 처리)
        vertexIndices.push_back(vIndex - 1);
        uvIndices.push_back(uvIndex - 1);
        normalIndices.push_back(nIndex - 1);

        // 다음 토큰 가져오기
        token = strtok(nullptr, " ");
    }

    // 삼각형이 아니면 오류 처리
    if (vertexIndices.size() != 3)
    {
        LOGE("Face does not contain exactly 3 vertices!");
        return false;
    }

    // 정확히 3개의 정점을 vecFaceIndex에 추가
    objMeshModel.vecFaceIndex.push_back(FaceIndex(vertexIndices[0], uvIndices[0], normalIndices[0]));
    objMeshModel.vecFaceIndex.push_back(FaceIndex(vertexIndices[1], uvIndices[1], normalIndices[1]));
    objMeshModel.vecFaceIndex.push_back(FaceIndex(vertexIndices[2], uvIndices[2], normalIndices[2]));

    return true;
}



bool OBJMesh::CalculateNormal(bool flatShading)
{
    if (objMeshModel.normals.empty()) {
        objMeshModel.normals.resize(objMeshModel.positions.size(), vec3(0.0f, 0.0f, 0.0f));

        if (objMeshModel.vecFaceIndex.size() % 3 != 0) {
            LOGI("Error: Face index size is not a multiple of 3! Size: %d", (int)objMeshModel.vecFaceIndex.size());
            return false;
        }

        for (size_t i = 0; i < objMeshModel.vecFaceIndex.size(); i += 3) {
            if (i + 2 >= objMeshModel.vecFaceIndex.size()) {
                LOGI("Error: Index out of range! i: %d, size: %d", (int)i, (int)objMeshModel.vecFaceIndex.size());
                return false;
            }

            int idx0 = objMeshModel.vecFaceIndex[i].vertexIndex;
            int idx1 = objMeshModel.vecFaceIndex[i + 1].vertexIndex;
            int idx2 = objMeshModel.vecFaceIndex[i + 2].vertexIndex;

            if (idx0 >= objMeshModel.positions.size() || idx1 >= objMeshModel.positions.size() || idx2 >= objMeshModel.positions.size()) {
                LOGI("Error: Position index out of bounds! idx0: %d, idx1: %d, idx2: %d, size: %d",
                     idx0, idx1, idx2, (int)objMeshModel.positions.size());
                return false;
            }

            vec3 a = objMeshModel.positions[idx0];
            vec3 b = objMeshModel.positions[idx1];
            vec3 c = objMeshModel.positions[idx2];

            vec3 faceNormal = cross(b - a, c - a);
            if (length(faceNormal) == 0.0f) {
                faceNormal = vec3(0.0f, 1.0f, 0.0f);
            } else {
                faceNormal = normalize(faceNormal);
            }

            if (idx0 >= objMeshModel.normals.size() || idx1 >= objMeshModel.normals.size() || idx2 >= objMeshModel.normals.size()) {
                LOGI("Error: Normal index out of bounds! idx0: %d, idx1: %d, idx2: %d, size: %d",
                     idx0, idx1, idx2, (int)objMeshModel.normals.size());
                return false;
            }

            if (flatShading) {
                // Flat shading: 각 정점에 같은 법선 벡터 적용 (면 단위로 고정된 법선 사용)
                objMeshModel.normals[idx0] = faceNormal;
                objMeshModel.normals[idx1] = faceNormal;
                objMeshModel.normals[idx2] = faceNormal;
            } else {
                // Smooth shading: 법선을 누적한 후 정규화 (각 정점에서 부드러운 쉐이딩)
                objMeshModel.normals[idx0] += faceNormal;
                objMeshModel.normals[idx1] += faceNormal;
                objMeshModel.normals[idx2] += faceNormal;

            }
        }

        if (!flatShading) {
            // Smooth shading의 경우, 모든 정점의 법선을 정규화
            for (auto& normal : objMeshModel.normals) {
                normal = normalize(normal);
            }
        }
    }
    return true;
}


void OBJMesh::CreateInterleavedArray()
{
    objMeshModel.vertices.resize(objMeshModel.vecFaceIndex.size());
    objMeshModel.indices.resize(objMeshModel.vecFaceIndex.size());

    // Get the total number of indices.
    IndexCount = (int)objMeshModel.indices.size();

    for (size_t i = 0; i < objMeshModel.vecFaceIndex.size(); i++) {
        int index = objMeshModel.vecFaceIndex[i].vertexIndex;
        objMeshModel.vertices[i].position = objMeshModel.positions[index];
        objMeshModel.indices[i] = static_cast<unsigned short>(index);

        if (!objMeshModel.uvs.empty()) {
            objMeshModel.vertices[i].uv = objMeshModel.uvs[objMeshModel.vecFaceIndex[i].uvIndex];
        }
        if (!objMeshModel.normals.empty()) {
            objMeshModel.vertices[i].normal = objMeshModel.normals[objMeshModel.vecFaceIndex[i].normalIndex];
        }
    }
}

bool OBJMesh::ClearMesh()
{
    objMeshModel.positions.clear();
    objMeshModel.normals.clear();
    objMeshModel.uvs.clear();
    objMeshModel.tangents.clear();
    objMeshModel.indices.clear();
    objMeshModel.vecFaceIndex.clear();
    return true;
}

bool OBJMesh::CalculateTangents()
{
    // LOGI("objMeshModel.uvs.size() %d", objMeshModel.uvs.size());

    if (objMeshModel.uvs.size() == 0) return false;

    // Mathematics for 3D Game Programming and Computer Graphics, 3rd edition
    // http://www.terathon.com/code/tangent.html
    vector<vec3> tan1Accum;
    vector<vec3> tan2Accum;

    objMeshModel.tangents.resize(objMeshModel.positions.size());

    for ( uint i = 0; i < objMeshModel.positions.size(); i++ ) {
        tan1Accum.push_back(vec3(0.0f));
        tan2Accum.push_back(vec3(0.0f));
        objMeshModel.tangents.push_back(vec4(0.0f));
    }

    int index0, index1, index2;
    int index0uv, index1uv, index2uv;

    // Compute the tangent vector
    for( uint i = 0; i < objMeshModel.vecFaceIndex.size(); i += 3 )
    {
        index0 = objMeshModel.vecFaceIndex.at(i).vertexIndex;
        index1 = objMeshModel.vecFaceIndex.at(i+1).vertexIndex;
        index2 = objMeshModel.vecFaceIndex.at(i+2).vertexIndex;

        const vec3 &p0 = objMeshModel.positions.at(index0);
        const vec3 &p1 = objMeshModel.positions.at(index1);
        const vec3 &p2 = objMeshModel.positions.at(index2);

        index0uv = objMeshModel.vecFaceIndex.at(i).uvIndex;
        index1uv = objMeshModel.vecFaceIndex.at(i+1).uvIndex;
        index2uv = objMeshModel.vecFaceIndex.at(i+2).uvIndex;

        const vec2 &tc1 = objMeshModel.uvs.at(index0uv);
        const vec2 &tc2 = objMeshModel.uvs.at(index1uv);
        const vec2 &tc3 = objMeshModel.uvs.at(index2uv);

        // Equation 1:
        // Q1 = P1 − P0
        // Q2 = P2 − P0

        vec3 q1 = p1 - p0;
        vec3 q2 = p2 - p0;

        // Equation 2:
        // (s1, t1) = (u1 − u0, v1 − v0)
        // (s2, t2) = (u2 − u0, v2 − v0)

        // Equation 3:
        // Q1 = s1T + t1B
        // Q2 = s2T + t2B

        float s1 = tc2.x - tc1.x, s2 = tc3.x - tc1.x;
        float t1 = tc2.y - tc1.y, t2 = tc3.y - tc1.y;

        // Equation 4:
        // [ (Q1)x   (Q1)y  (Q1)z ]       [(s1,  t1)] [(Tx   Ty  Tz)]
        //                            =
        // [ (Q2)x   (Q2)y  (Q2)z ]       [(s2,  t2)] [(Bx   By  Bz)]

        // Equation 5:
        // [(Tx   Ty  Tz)]      [      1       ]  [(s1,  -t1)] [ (Q1)x   (Q1)y  (Q1)z ]
        //                  =   ----------------
        // [(Bx   By  Bz)]      [(-s1t2 - s2t1)]  [(-s2,  t2)] [ (Q2)x   (Q2)y  (Q2)z ]

        float r = 1.0f / (s1 * t2 - s2 * t1);

        vec3 tan1( (s1*q1.x - t1*q2.x) * r,
                   (s1*q1.y - t1*q2.y) * r,
                   (s1*q1.z - t1*q2.z) * r); // Tagent

        vec3 bTan( (t2*q2.x - s2*q1.x) * r,
                   (t2*q2.y - s2*q1.y) * r,
                   (t2*q2.z - s2*q1.z) * r); // BiTagent

        tan1Accum[index0] += tan1;
        tan1Accum[index1] += tan1;
        tan1Accum[index2] += tan1;

        tan2Accum[index0] += bTan;
        tan2Accum[index1] += bTan;
        tan2Accum[index2] += bTan;
    }

    for( uint i = 0; i < objMeshModel.positions.size(); ++i )
    {
        objMeshModel.tangents[i] = vec4(glm::normalize( tan1Accum[i] ),1.0);
        // It is advise to store the BiTangents into an array also instead of calclating at fly time in vertex shader.
        LOGI("\nT: %f, %f, %f", objMeshModel.tangents[i].x, objMeshModel.tangents[i].y, objMeshModel.tangents[i].z);
    }

    for(int i = 0; i < objMeshModel.vecFaceIndex.size(); i++)
    {
        int index = objMeshModel.vecFaceIndex.at(i + 0).vertexIndex;
        objMeshModel.vertices[i].tangent = objMeshModel.tangents.at(index);
        LOGI("\nP: %f, %f, %f", objMeshModel.vertices[i].tangent.x, objMeshModel.vertices[i].tangent.y, objMeshModel.vertices[i].tangent.z);
    }

    tan1Accum.clear();
    tan2Accum.clear();

    return true;
}

Mesh* OBJMesh::ParseObjModel(AAssetManager* assetManager, const char* path, bool flatShading)
{
    if (!ParseFileInfo(assetManager, path)) {
        return nullptr;
    }
    CreateInterleavedArray();
    CalculateNormal(flatShading);
    CalculateTangents();

    // PrintModelInfo();
    ClearMesh();
    return &objMeshModel;
}

void OBJMesh::PrintModelInfo() {
    LOGI("vertices size : %d", objMeshModel.vertices.size());
    for (Vertex &v : objMeshModel.vertices) {
        LOGI("vertex.position : %f %f %f", v.position.x,v.position.y, v.position.z );
    }
    /*
    for (Vertex &v : objMeshModel.vertices) {
        LOGI("uv : %f %f", v.uv.x, v.uv.y);
    }
    for (Vertex &v : objMeshModel.vertices) {
        LOGI("normal : %f %f %f", v.normal.x, v.normal.y, v.normal.z);
    }
    for (Vertex &v : objMeshModel.vertices) {
        LOGI("tan : %f %f %f", v.tangent.x, v.tangent.y, v.tangent.z);
    }
    */
}
