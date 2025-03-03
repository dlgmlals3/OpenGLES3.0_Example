#version 300 es

layout(location = 0) in vec4 VertexPosition;
layout(location = 1) in vec4 VertexColor;
layout(location = 2) in mat4 MODELVIEWPROJECTIONMATRIX;
/*
layout(location = 2) in vec4 MVP_Row0;
layout(location = 3) in vec4 MVP_Row1;
layout(location = 4) in vec4 MVP_Row2;
layout(location = 5) in vec4 MVP_Row3;
*/
//uniform mat4 MODELVIEWPROJECTIONMATRIX;


out vec4 Color;

void main() 
{
  gl_Position = MODELVIEWPROJECTIONMATRIX * VertexPosition;
  Color = VertexColor;
}