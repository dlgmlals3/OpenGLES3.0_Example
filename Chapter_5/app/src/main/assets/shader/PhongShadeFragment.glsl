#version 300 es
precision mediump float;

// Material property
uniform vec3    MaterialAmbient;
uniform vec3    MaterialSpecular;
uniform vec3    MaterialDiffuse;

// Light property
uniform vec3    LightAmbient;
uniform vec3    LightSpecular;
uniform vec3    LightDiffuse;
uniform float   ShininessFactor;

uniform vec3    LightPosition;

in vec3    normalByView;
in vec3    vertexByView;

layout(location = 0) out vec4 FinalColor;

vec3 PhongShading()
{
    vec3 normalizeNormal, normalizeVertex, normalizeLightVec, V, R, ambient, diffuse, specular;
    float sIntensity, cosAngle;

    normalizeNormal   = normalize( normalByView );
    normalizeVertex = normalize( vertexByView );
    normalizeLightVec = normalize( LightPosition - vertexByView );

    // Diffuse Intensity
    cosAngle = max( 0.0, dot( normalizeNormal, normalizeLightVec ));

    // Specular Intensity
    // dlgmlals3 이게 왜 viewer야 ?? 걍 버텍스 뷰좌표계잖아
    // 즉, 카메라(eye)는 원점 (0,0,0) 에 위치한다고 가정해요.
    V = -normalizeVertex; // Viewer's vector

    R = reflect( -normalizeLightVec, normalizeNormal ); // Reflectivity
    sIntensity 	= pow( max( 0.0, dot( R, V ) ), ShininessFactor );

    // ADS color as result of Material & Light interaction
    ambient    = MaterialAmbient  * LightAmbient;
    diffuse    = MaterialDiffuse  * LightDiffuse;
    specular   = MaterialSpecular * LightSpecular;

    return ambient + ( cosAngle * diffuse ) + ( sIntensity * specular );
}

void main() {
    FinalColor = vec4(PhongShading(), 1.0);
}
