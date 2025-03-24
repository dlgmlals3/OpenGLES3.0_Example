#version 300 es
// Vertex information
layout(location = 0) in vec4  VertexPosition;
layout(location = 1) in vec3  Normal;

// Model View Project matrix
uniform mat4    ModelViewProjectionMatrix;
uniform mat4    ModelViewMatrix;
uniform mat3    NormalMatrix;

out vec3 normalByView;
out vec3 vertexByView;

void main() 
{
    normalByView = NormalMatrix * Normal;
    vertexByView    = vec3 ( ModelViewMatrix * VertexPosition );
    gl_Position = ModelViewProjectionMatrix * VertexPosition;
}