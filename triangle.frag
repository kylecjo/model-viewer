#version 450 core

uniform vec3 pointLightPos;
uniform vec3 pointLightCol;
uniform vec3 ambient;
uniform vec3 cameraPos;

in vec3 vertexColour;
in vec3 Normal;
in vec3 fragPos;

out vec4 fragColour;

float specularStrength = 0.5;

// as adapted from https://learnopengl.com/Lighting/Basic-Lighting
void main()
{
    //diffuse calculations
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(pointLightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * pointLightCol;

    //specular calculations
    vec3 viewDir = normalize(cameraPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * pointLightCol;

    //diffuse shading
    //vec3 result = (ambient + diffuse + specular) * vertexColour;
    
   //specular shading
    vec3 result = (ambient + diffuse + specular) * vertexColour;
    
    fragColour = vec4(result, 1.0);
}
