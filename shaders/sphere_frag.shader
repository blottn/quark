#version 330

out vec4 fragColor;

uniform sampler2D tex;

in vec3 coord;
in vec3 LightIntensity;

void main(){
    vec2 texCoord = vec2(coord.x, coord.y);
    vec4 texture = texture(tex, texCoord);
	fragColor = texture * vec4(LightIntensity, 1.0);
}
