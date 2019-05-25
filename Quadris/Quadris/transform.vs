#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 Color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 color;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
	vec2 aux_aTexCoord = (model * vec4(aTexCoord.x, aTexCoord.y, 0.0, 0.0)).xy;
    TexCoord = vec2(aux_aTexCoord.x, 1.0 - aux_aTexCoord.y);
	// TexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
	Color = color;
}