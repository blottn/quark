#version 330

out vec4 fragColor;

in vec3 LightIntensity;

uniform sampler2D tex;

void main(){
    vec4 textureVal = vec4(0.0,0.0,1,1.0);
	fragColor = textureVal;// * vec4 (LightIntensity, 1.0);
}
