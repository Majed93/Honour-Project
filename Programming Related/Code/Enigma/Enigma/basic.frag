// Minimal fragment shader

#version 420

uniform sampler2D Texture;
in vec2 Frag_UV;
in vec4 Frag_Colour;
in vec4 fcolour;
out vec4 outputColor;
void main()
{
	//outputColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
	outputColor = fcolour;//Frag_Colour * texture(Texture, Frag_UV.st);
}