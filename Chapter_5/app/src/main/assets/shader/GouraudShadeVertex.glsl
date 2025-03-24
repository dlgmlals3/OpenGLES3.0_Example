#version 300 es

// Vertex information
layout(location = 0) in vec4  VertexPosition;
layout(location = 1) in vec3  Normal;

// Model View Project matrix
uniform mat4    ModelViewProjectionMatrix;
uniform mat4    ModelViewMatrix;
uniform mat3    NormalMatrix;

// Material property
uniform vec3    MaterialAmbient;
uniform vec3    MaterialSpecular;
uniform vec3    MaterialDiffuse;
uniform float   ShininessFactor;

// Light property
uniform vec3    LightAmbient;
uniform vec3    LightSpecular;
uniform vec3    LightDiffuse;
uniform vec3    LightPosView;

out vec4 FinalColor;

vec3 GouraudShading()
{
    vec3 nNormal, vertexViewPos, nLight, V, R, ambient, diffuse, specular;
    float sIntensity, ndotl;

    nNormal   = normalize ( NormalMatrix * Normal );
    vertexViewPos  = vec3 ( ModelViewMatrix * VertexPosition );
    nLight    = normalize( LightPosView - vertexViewPos );

    // Diffuse Intensity
    ndotl = max( 0.0, dot( nNormal, nLight ));

    // Specular Intensity
    V       = normalize( -vertexViewPos );
    R       = reflect( -nLight, nNormal );
    //R = (2.0 * nNormal * dot(nNormal, nLight)) - nLight;

    sIntensity = pow( max(0.0, dot(R, V) ), ShininessFactor);

    // ADS color as result of Material & Light interaction
    ambient = MaterialAmbient * LightAmbient; //Ambient light
    diffuse = MaterialDiffuse * LightDiffuse; //Diffuse light
    specular = MaterialSpecular * LightSpecular; //Specular light

    return ambient + (ndotl * diffuse) + (sIntensity * specular);
}

void main()
{
    FinalColor = vec4(GouraudShading(), 1);
    gl_Position = ModelViewProjectionMatrix * VertexPosition;
}
