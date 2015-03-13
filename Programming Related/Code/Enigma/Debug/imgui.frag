// Minimal fragment shader

#version 420

uniform sampler2D Texture;
in vec2 Frag_UV;
in vec4 Frag_Colour;
out vec4 outputColor;
void main()
{
	outputColor = Frag_Colour * texture(Texture, Frag_UV.st);
}