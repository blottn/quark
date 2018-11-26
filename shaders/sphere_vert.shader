#version 330

in vec3 vertex_position;

out vec3 LightIntensity;
out vec3 coord;

vec4 LightPosition = vec4 (0.0, 0.0, 0.0, 1.0); // Light position in world coords.
vec3 Kd = vec3 (1.0, 1.0, 1.0); // green diffuse surface reflectance
vec3 Ld = vec3 (1.0, 1.0, 1.0); // Light source intensity


uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

uniform int bright;

void main(){
  coord = normalize(vertex_position);
  mat4 ModelViewMatrix = view * model;
  mat3 NormalMatrix =  mat3(ModelViewMatrix);
  // Convert normal and position to eye coords
  // Normal in view space
  vec3 tnorm = normalize( NormalMatrix * normalize(vertex_position));
  // Position in view space
  vec4 eyeCoords = ModelViewMatrix * vec4(vertex_position,1.0);
  //normalised vector towards the light source
  vec3 s = normalize(vec3(LightPosition - eyeCoords));
  
  // The diffuse shading equation, dot product gives us the cosine of angle between the vectors
  if (bright == 1) {
    LightIntensity = vec3(1.0,1.0,1.0);
  }
  else {
    LightIntensity = Ld * Kd * max( dot( s, tnorm ), 0.0 );
  }
  
  // Convert position to clip coordinates and pass along
  gl_Position = proj * view * model * vec4(vertex_position,1.0);

}


  
