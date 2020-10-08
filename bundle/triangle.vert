#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
//layout(location = 2) in vec3 texture;

uniform mat4 model;
uniform mat4 proj;
uniform mat4 view;
uniform vec3 colour;

out vec3 vertexColour;
out vec3 Normal;
out vec3 fragPos;

void main()
{
    gl_Position =  proj * view * model * vec4(position, 1.0);
    fragPos = vec3(model * vec4(position, 1.0));
    vertexColour = colour;
    Normal = normal;
}
