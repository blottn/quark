#version 330

in vec3 vertex_position;

out vec3 norm;
out vec3 FragPos;

out vec2 TexCoord;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

void main(){
  // Convert position to clip coordinates and pass along
  float PI = 3.1415926535897932384626433832795028841971;
  TexCoord = vec2(1.0,1.0);
// UV mapping for sphere
  TexCoord.x = 0.5 + (atan(normalize(vertex_position).x,normalize(vertex_position).z) / (2*PI));
  TexCoord.y = 0.5 - (asin(normalize(vertex_position).y) / PI);
  norm = mat3(transpose(inverse(model))) * normalize(vertex_position);
  gl_Position = proj * view * model * vec4(vertex_position,1.0);
  FragPos = vec3(model * vec4(vertex_position, 1.0));
}


  
