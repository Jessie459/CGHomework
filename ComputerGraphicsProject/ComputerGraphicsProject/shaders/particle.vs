#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
out vec4 ParticleColor;

uniform vec3 offset;
uniform vec4 color;
uniform float size;

void main()
{
	gl_Position = vec4(aPos * size + offset, 1.0f);
	TexCoord = aTexCoord;
	ParticleColor = color;
}