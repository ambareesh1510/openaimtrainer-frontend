#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add your custom variables here

// Input lighting values
uniform vec4 ambient;
uniform vec3 viewPos;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);

    vec4 tint = colDiffuse * fragColor;

    // vec3 light = normalize(viewPos - fragPosition);
    vec3 light = normalize(viewPos);
    float NdotL = max(dot(normal, light), 0.0);
    lightDot += vec3(1.0)*NdotL;

    finalColor = (texelColor*((tint + vec4(specular, 1.0))*vec4(lightDot, 1.0)));
    // finalColor += texelColor*(ambient/10.0)*tint;

    // Gamma correction
    finalColor = pow(finalColor, vec4(1.0/2.2));
}
