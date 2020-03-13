#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;

uniform mat4 model;
uniform mat4 proj;
uniform mat4 view;
//add 2 more transformation matrices

out vec3 vertexColour;

void main()
{
    gl_Position =  proj * view * model * vec4(position, 1.0);
    vertexColour = colour;
}
