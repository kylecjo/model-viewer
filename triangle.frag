#version 450 core

uniform vec3 pointLightPos;
uniform vec3 pointLightCol;
uniform vec3 directionalDir;
uniform vec3 directionalCol;
uniform vec3 ambient;
uniform vec3 cameraPos;
uniform int specularFlag;

in vec3 vertexColour;
in vec3 Normal;
in vec3 fragPos;

out vec4 fragColour;

float specularStrength = 0.5;

// as adapted from https://learnopengl.com/Lighting/Basic-Lighting
void main()
{

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(pointLightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    float diffDir = max(dot(norm, directionalDir), 0.0);

    //diffuse pointLight
    vec3 diffuse = diff * pointLightCol;
    //directionalLight
    //diffuseDir = diffDir * directionalCol;

    vec3 viewDir = normalize(cameraPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 reflectDirDir = reflect(-directionalDir, norm);
    float specDir = pow(max(dot(viewDir, reflectDirDir), 0.0), 32);

    //specular pointLight
    vec3 specular = specularStrength * spec * pointLightCol;
    //specular pointLight + directional
    //vec3 specular = specularStrength * (specDir* directionalCol + spec * pointLightCol);

    vec3 result;

    if(specularFlag == 1){        
        //specular shading
        result = (ambient + diffuse + specular) * vertexColour;
    } else{
        //diffuse shading
        result = (ambient + diffuse) * vertexColour;
    }
    
    fragColour = vec4(result, 1.0);
}
