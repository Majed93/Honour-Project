// Minimal fragment shader
// This does majority of the work for lighting
#version 420

// Global constants (for this vertex shader)
vec4 specular_colour = vec4(1.0, 0.8, 0.9, 1.0);
vec4 global_ambient = vec4(0.5, 0.2, 0.1, 0.9);
float  shininess = 8;

// Inputs from the vertex shader
in vec3 fnormal, flightdir, fposition, fviewdir;
in vec4 fdiffusecolour, fambientcolour, fcolour;
//in float distancetolight;

uniform uint colourmode, emitmode;

// Output pixel fragment colour
out vec4 outputColor;


void main()
{
	
		specular_colour = vec4(0.508273, 0.508273, 0.508273, 1.0);
		global_ambient = vec4(0.19225, 0.19225, 0.19225, 1.0);
		shininess = 0.9 * 128; //SILVER

		vec4 emissive = vec4(0);				// Create a vec4(0, 0, 0) for our emmissive light
		vec4 fambientcolour = fdiffusecolour * 0.99;
		vec4 fspecularcolour =  vec4(1.0, 1.0, 0.7, 1.0);
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
		if (emitmode == 1) emissive = vec4(0.8, 0.8, 0.6, 0.95); 

		// Calculate the output colour, includung attenuation on the diffuse and specular components
		// Note that you may want to exclude the ambient form the attenuation factor so objects
		// are always visible, or include a global ambient



		outputColor = attenuation*(fambientcolour + diffuse + specular) + emissive + global_ambient;

}

