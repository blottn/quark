#version 330
out vec4 fragColor;

uniform sampler2D tex;

//uniform mat4 model;
//uniform mat4 proj;
//uniform mat4 view;

uniform int bright;

in vec3 coord;
in vec3 norm;
in vec3 FragPos;  

vec3 lPos = vec3(0.0, 0.0, 0.0); // Light position in world coords.
vec3 lColor = vec3 (1.0, 1.0, 1.0); // green diffuse surface reflectance
vec3 Ld = vec3 (1.0, 1.0, 1.0); // Light source intensity


void main(){

    vec3 LightIntensity = vec3(1.0,1.0,1.0);
    vec3 ambient = vec3(0.1,0.1,0.1);
    
    vec3 lightDir = normalize(lPos-FragPos);
    float diffusion = max(dot(normalize(norm), lightDir), 0.0);
    vec3 diffuse = diffusion * lColor;
    LightIntensity = ambient + diffuse;
    
    vec2 texCoord = vec2(coord.x, coord.y);
    vec4 texture = texture(tex, texCoord);


	fragColor = texture * vec4(LightIntensity, 1.0);
}
