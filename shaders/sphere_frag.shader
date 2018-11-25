#version 330

out vec4 fragColor;

uniform sampler2D tex;

in vec3 LightIntensity;

void main(){
    vec3 texCoord = vec3(0,0,0);
    vec4 texture = texture(tex, texCoord);
	fragColor = vec4(LightIntensity, 1.0);
}
