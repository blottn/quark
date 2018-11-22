#version 330 core
in vec3 pos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = pos;
    vec4 aPos = projection * view * vec4(pos, 1.0);
    gl_Position = aPos.xyww;
}
