#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 proj;
uniform mat4 view;
uniform vec3 colour;
uniform vec3 ambient;

out vec3 vertexColour;
out vec3 Normal;

void main()
{
    gl_Position =  proj * view * model * vec4(position, 1.0);
    vertexColour = ambient * colour;
    Normal = normal;
}
