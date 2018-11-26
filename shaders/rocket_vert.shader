#version 330

in vec3 vertex_position;
in vec3 vertex_normal;

//out vec3 coord;
out vec3 norm;
out vec3 FragPos;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

void main(){
  // Convert position to clip coordinates and pass along
  //coord = normalize(vertex_position);
  norm = mat3(transpose(inverse(model))) * normalize(vertex_position);
  gl_Position = proj * view * model * vec4(vertex_position,1.0);
  FragPos = vec3(model * vec4(vertex_position, 1.0));
}


  
