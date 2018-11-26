#version 330

out vec4 fragColor;

in vec3 LightIntensity;

uniform sampler2D tex;

void main(){
    vec4 textureVal = vec4(0.85,0.5,0,0.6);
	fragColor = textureVal;// * vec4 (LightIntensity, 1.0);
}
