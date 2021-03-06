//Majed Monem 2014/15 Graphical Enigma Simulator Honours Project

#version 330
in vec2 Position;
uniform mat4 ortho;
in vec2 UV;
in vec4 Colour;
out vec2 Frag_UV;
out vec4 Frag_Colour;
void main()
{
	Frag_UV = UV;
	Frag_Colour = Colour;
	gl_Position = ortho*vec4(Position.xy,0,1);
}
