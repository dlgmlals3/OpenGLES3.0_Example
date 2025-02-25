#version 310 es

layout(location = 0) in vec4 VertexPosition;
layout(location = 1) in vec4 VertexColor;

out vec4 Color;

// Uniform Block Declaration
uniform Transformation {
    mat4 ModelMatrix;
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
} Obj1;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
    gl_Position = Obj1.ProjectionMatrix * Obj1.ViewMatrix * Obj1.ModelMatrix * VertexPosition;
    //gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * VertexPosition;
    Color = VertexColor;
}