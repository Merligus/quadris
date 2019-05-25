#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Color;

uniform bool filled;
// texture samplers
uniform sampler2D text1;
uniform sampler2D text2;

void main()
{
	if(filled)
		FragColor = mix(texture(text1, TexCoord), vec4(Color, 1.0f), 0.5);
	else
		FragColor = texture(text2, TexCoord);
}