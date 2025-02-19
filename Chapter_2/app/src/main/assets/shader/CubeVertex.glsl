attribute vec4 VertexPosition;
attribute vec4 VertexColor;
varying vec4 VarColor;

void main() 
{
  gl_Position = VertexPosition;
  VarColor = VertexColor;
}