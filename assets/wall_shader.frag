#version 330

in vec3 fragPosition;  // World-space position of the fragment
in vec3 fragNormal;

uniform vec4 ambient;
uniform vec3 viewPos;
uniform vec4 colDiffuse;

out vec4 finalColor;

uniform float squareSize;         // Size of one square tile

void main() {
    vec3 scaled = fragPosition / 0.5;

    int x = int(floor(scaled.x));
    int y = int(floor(scaled.y));
    int z = int(floor(scaled.z));

    int checker = (x + y + z) & 1;

    vec3 color = (checker == 0) ? vec3(0.608, 0.678, 0.718) : vec3(1);
    vec4 fragColor = vec4(color, 1.0);

    vec4 texelColor = fragColor;
    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);

    vec4 tint = vec4(1);

    vec3 light = normalize(viewPos - fragPosition);
    float dist = length(viewPos - fragPosition);
    float NdotL = max(dot(normal, light), 0.7);
    lightDot += vec3(0.8)*NdotL * min(pow(dist / 10, -2), 1.0);

    finalColor = (texelColor*((tint + vec4(specular, 1.0))*vec4(lightDot, 1.0)));
    finalColor += texelColor*(ambient)*tint;

    // Gamma correction
    finalColor = pow(finalColor, vec4(1.0/2.2));
    return;
}

