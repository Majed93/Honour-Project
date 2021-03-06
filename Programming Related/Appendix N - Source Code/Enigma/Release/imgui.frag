//Majed Monem 2014/15 Graphical Enigma Simulator Honours Project

// Minimal fragment shader

#version 330

uniform sampler2D Texture;
in vec2 Frag_UV;
in vec4 Frag_Colour;
out vec4 outputColor;
void main()
{
	outputColor = Frag_Colour * texture(Texture, Frag_UV.st);
}