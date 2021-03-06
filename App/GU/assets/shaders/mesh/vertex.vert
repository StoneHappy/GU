#version 330 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;
layout (std140) uniform Camera
{
   mat4 ProjectionView;
}; 
void main()
{
   gl_Position = ProjectionView * vec4(a_Position, 1.0);
}