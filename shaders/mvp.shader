#version 330

in vec3 vertex_position;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

void main(){
   // gl_Position = vec4(vertex_position, 1.0); 
    gl_Position = proj * view * model * vec4(vertex_position,1.0);
}


  
