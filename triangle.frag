#version 450 core

uniform vec3 pointLightPos;
uniform vec3 pointLightCol;
uniform vec3 ambient;

in vec3 vertexColour;
in vec3 Normal;
in vec3 fragPos;

out vec4 fragColour;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(pointLightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * pointLightCol;
    vec3 result = (ambient + diffuse) * vertexColour;
    fragColour = vec4(result, 1.0);
}
