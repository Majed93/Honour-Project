//Majed Monem 2014/15 Graphical Enigma Simulator Honours Project
// Minimal fragment shader
// This does majority of the work for lighting
#version 400

// Global constants (for this vertex shader)
vec4 specular_colour = vec4(0.1, 0.1, 0.1, 1.0);
vec4 global_ambient = vec4(0.1, 0.1, 0.1, 0.9);
float  shininess = 8;

// Inputs from the vertex shader
in vec3 fnormal, flightdir, fposition, fviewdir;
in vec4 fdiffusecolour, fambientcolour, fcolour;
//in float distancetolight;
in vec2 ftexcoord;	// This is the rasterized texture coordinate
uniform sampler2D tex1;	// This is the texture object

uniform uint colourmode, emitmode, comp, textmode;
//varying vec2 v_texcoord;
// Output pixel fragment colour
out vec4 outputColor;


void main()
{

		// Extract the texture colour to colour our pixel
		vec4 texcolour = texture(tex1, ftexcoord);


		switch (comp)
		{
			case 0:
				specular_colour = vec4(0.508273, 0.508273, 0.508273, 1.0);
				global_ambient = vec4(0.19225, 0.19225, 0.19225, 1.0);
				shininess = 0.4 * 128; //SILVER
				break;

			case 1:
				specular_colour = vec4(0.4508273, 0.4508273, 0.4508273, 1.0);
				global_ambient = vec4(0.15225, 0.15225, 0.15225, 1.0);
				shininess = 0.4 * 128; //DARKER SILVER
				break;

			case 2:
				specular_colour = vec4(0.332741, 0.328634, 0.346435, 1.0);
				global_ambient = vec4(0.05375, 0.05, 0.06625, 1.0);
				shininess = 0.9 * 128; //OBSIDIAN
				break;

			case 3:
				specular_colour = vec4(0.774597, 0.774597, 0.774597, 1.0);
				global_ambient = vec4(0.25, 0.25, 0.25, 1.0);
				shininess = 0.6 * 128; //CHROME
				break;

			case 4:
				specular_colour = vec4(0.628281, 0.555802, 0.366065, 1.0);
				global_ambient = vec4(0.24725, 0.1995, 0.0745, 1.0);
				shininess = 0.4 * 128; //GOLD
				break;

			case 5:
				specular_colour = vec4(0.628281, 0.555802, 0.366065, 1.0);
				global_ambient = vec4(0.24725, 0.1995, 0.0745, 1.0);
				shininess = 0.9 * 128; //GOLD
				break;

			case 6:
				specular_colour = vec4(0.256777, 0.137622, 0.086014, 1.0);
				global_ambient = vec4(0.19125, 0.0735, 0.0225, 1.0);
				shininess = 0.1 * 128; //COPPER
				break;

			case 7:
				specular_colour = vec4(0.628281, 0.555802, 0.366065, 1.0);
				global_ambient = vec4(0.24725, 0.1995, 0.0745, 1.0);
				shininess = 0.9 * 128; //GOLD
				break;

			case 8:
				specular_colour = vec4(0.4508273, 0.4508273, 0.4508273, 1.0);
				global_ambient = vec4(0.19225, 0.19225, 0.19225, 1.0);
				shininess = 0.4 * 128; //SILVER
				break;

			case 9:
				specular_colour = vec4(0.4, 0.4, 0.4, 1.0);
				global_ambient = vec4(0.0, 0.0, 0.0, 1.0);
				shininess = 0.25 * 128; //BLACK PLASTIC
				break;
			
			case 10:
				specular_colour = vec4(0.6774597, 0.6774597, 0.6774597, 1.0);
				global_ambient = vec4(0.2, 0.2, 0.2, 1.0);
				shininess = 0.6 * 128; //CHROME
				break;
			
			case 11:
				specular_colour = vec4(0.16774597, 0.16774597, 0.6774597, 1.0);
				global_ambient = vec4(0.2, 0.2, 0.2, 1.0);
				shininess = 0.4 * 128; //CHROME
				break;

			case 12:
				specular_colour = vec4(0.6774597, 0.6774597, 0.6774597, 1.0);
				global_ambient = vec4(0.5, 0.5, 0.5, 1.0);
				shininess = 0.9 * 128; //CHROME
				break;

			case 13:
				specular_colour = vec4(0.6774597, 0.6774597, 0.6774597, 1.0);
				global_ambient = vec4(0.2, 0.2, 0.2, 1.0);
				shininess = 0.9 * 128; //CHROME
				break;
			default:
				specular_colour = vec4(0.908273, 0.908273, 0.908273, 1.0);
				global_ambient = vec4(0.409225, 0.409225, 0.409225, 1.0);
				shininess = 0.9 * 128; //SILVER

				break;
		}
	
		vec4 emissive = vec4(0);				// Create a vec4(0, 0, 0) for our emmissive light
		vec4 fambientcolour = fdiffusecolour * 3.99;
		vec4 fspecularcolour =  vec4(0.8, 0.8, 0.55, 1.0);

		if(textmode == 1)
		{
				specular_colour = vec4(1.0, 0.908273, 0.908273, 1.0);
				global_ambient = vec4(0.1, 0.1, 0.1, 1.0);
				shininess = 0.9 * 128;
				fambientcolour = fdiffusecolour * 8.9;
				fspecularcolour =  vec4(1.0, 0.8, 0.0, 1.0);
		}
		float distancetolight = length(flightdir);

		// Normalise interpolated vectors
		vec3 L = normalize(flightdir);
		vec3 N = normalize(fnormal);		

		// Calculate the diffuse component
		vec4 diffuse = max(dot(N, L), 0.0) * fdiffusecolour;

		// Calculate the specular component using Phong specular reflection
		vec3 V = normalize(-fposition.xyz);	
		vec3 R = reflect(-L, N);
		vec4 specular = pow(max(dot(R, V), 0.0), shininess) * fspecularcolour;

		// Calculate the attenuation factor;
		float attenuation_k1 = 0.7;
		float attenuation_k2 = 0.7;
		float attenuation_k3 = 0.7;
		float attenuation = 1.0 / (attenuation_k1 + attenuation_k2*distancetolight + attenuation_k3 * pow(distancetolight, 2));
	
		// simple hack to make the light brighter, it would be better to change the attenuation equation!
		attenuation *= 1.5;

		// If emitmode is 1 then we enable emmissive lighting
		//if (emitmode == 1) emissive = vec4(0.8, 0.8, 0.8, 1.0); 

		//if (emitmode == 2) emissive = vec4(0.05, 0.05, 0.99, 1.0); 

		// Calculate the output colour, includung attenuation on the diffuse and specular components
		// Note that you may want to exclude the ambient form the attenuation factor so objects
		// are always visible, or include a global ambient

		if (textmode == 1)
		{
		
			vec4 texture_diffuse = (fambientcolour + diffuse) * texcolour;
			outputColor = attenuation*(texture_diffuse + specular) + emissive + global_ambient;
		}
		else
		{
			outputColor = attenuation*(fambientcolour + diffuse + specular) + emissive + global_ambient;
		}
}

