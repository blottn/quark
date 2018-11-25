#version 330

out vec4 fragColor;

in vec3 LightIntensity;
void main(){
	fragColor = vec4 (LightIntensity, 1.0);
}
