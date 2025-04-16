#version 300 es

//Define Constant for Wobble Shader
#define AMPLITUDE 1.2
#define PI 3.141592
#define RIPPLE_AMPLITUDE 0.05
#define PREQUENCY 5.0

// Vertex information
layout(location = 0) in vec4  VertexPosition;
layout(location = 1) in vec3  Normal;

// Model View Project Normal matrix
uniform mat4    ModelViewProjectionMatrix;
uniform mat4    ModelViewMatrix;
uniform mat3    NormalMatrix;

// Timer
uniform float   Time;

// Output variable for fragment shader
out vec3    normalByView;
out vec3    positionByView;

vec4 wabbleEffect() {
    vec4 position = VertexPosition;
    position.y += sin(position.x + Time) * AMPLITUDE;
    return ModelViewProjectionMatrix * position;
}

vec4 rippleEffect() {
    vec4 position = VertexPosition;
    float distance = length(position);
    position.y = sin(2.0 * PI * distance * PREQUENCY + Time) * RIPPLE_AMPLITUDE;
    return ModelViewProjectionMatrix * position;
}

void main()
{
    normalByView   = normalize ( NormalMatrix * Normal );
    positionByView  = vec3 ( ModelViewMatrix * VertexPosition );
    gl_Position = rippleEffect();
}
