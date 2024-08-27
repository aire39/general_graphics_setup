#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0

uniform vec4 color; // specify a color output to the fragment shader
uniform mat4 ortho_mat; // specify a color output to the fragment shader
out vec4 vertexColor; // specify a color output to the fragment shader

void main()
{
    gl_Position = ortho_mat * vec4(aPos, 1.0); // see how we directly give a vec3 to vec4's constructor
    vertexColor = color;
}
