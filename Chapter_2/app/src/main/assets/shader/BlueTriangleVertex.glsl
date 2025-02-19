attribute vec4 VertexPosition;
attribute vec4 VertexColor;
uniform mat4 MODELVIEWPROJECTIONMATRIX;
varying vec4 VarColor;

void main()
{
  gl_Position = MODELVIEWPROJECTIONMATRIX * VertexPosition;
  VarColor = VertexColor;
}