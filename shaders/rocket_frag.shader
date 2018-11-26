#version 330
out vec4 fragColor;

uniform sampler2D tex;

uniform vec3 cameraPos;
uniform int bright;
uniform int shininess;

//in vec3 coord;
in vec3 norm;
in vec3 FragPos;  
//in vec2 TexCoord;

vec3 lPos = vec3(0.0, 0.0, 0.0); // Light position in world coords.
vec3 lColor = vec3 (1.0, 1.0, 1.0); // green diffuse surface reflectance
vec3 Ld = vec3 (1.0, 1.0, 1.0); // Light source intensity


void main(){

    vec3 LightIntensity = vec3(1.0,1.0,1.0);
    if (bright == 0) {
        vec3 ambient = vec3(0.1,0.1,0.1);
    
        vec3 lightDir = normalize(lPos-FragPos);
        float diffusion = max(dot(normalize(norm), lightDir), 0.0);
        vec3 diffuse = diffusion * lColor;

        float power = 0.4;
        vec3 cameraDir = normalize(cameraPos-FragPos);
        vec3 ref = reflect(-lightDir, normalize(norm));

        float specularity = pow(max(dot(cameraDir, ref), 0.0), 128);
        vec3 spec = power * specularity * lColor;
        LightIntensity = ambient + diffuse + spec;
    }
    vec4 texture = vec4(1.0,1.0,1.0,1.0);

	fragColor = texture * vec4(LightIntensity, 1.0);
}
