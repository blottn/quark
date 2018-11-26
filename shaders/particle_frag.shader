#version 330

out vec4 fragColor;

uniform sampler2D tex;

void main(){
    vec4 textureVal = vec4(0.85,0.5,0,0.5);
	fragColor = textureVal;
}
