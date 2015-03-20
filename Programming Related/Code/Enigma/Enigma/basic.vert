//Majed Monem 2014/15 Graphical Enigma Simulator Honours Project

// Specify minimum OpenGL version
#version 400

// Define the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
layout(location = 2) in vec3 normal;


// Outputs to send to the fragment shader
out vec3 fnormal;
out vec3 flightdir, fposition, fviewdir;
out vec4 fdiffusecolour, fcolour;

// These are the uniforms that are defined in the application
uniform mat4 model, view, projection;
uniform mat3 normalmatrix;
uniform uint colourmode, comp, wiremode;
uniform vec4 lightpos;
uniform highp float color1, color2, color3, color4;
void main()
{
	vec4 position_h = vec4(position, 1.0);	// Convert the (x,y,z) position to homogeneous coords (x,y,z,w)
	vec3 light_pos3 = lightpos.xyz;			
	vec4 newcolour = vec4(colour, 1.0);
	float num = wiremode;
	// Switch the vertex colour based on the colourmode
	if (colourmode == 1)
		fdiffusecolour = newcolour;
	else
		fdiffusecolour = vec4(0.50754, 0.50754, 0.50754, 1.0);
	
	switch (comp)
	{
		case 0:
			fdiffusecolour = vec4(0.50754, 0.50754, 0.50754, 1.0); //SILVER
			break;

		case 1:
			fdiffusecolour = vec4(0.40754, 0.40754, 0.40754, 1.0); //DARKER SILVER
			break;

		case 2:
			fdiffusecolour = vec4(0.12275, 0.11, 0.12525, 1.0); //OBSIDIAN
			break;

		case 3:
			fdiffusecolour = vec4(0.4, 0.4, 0.4, 1.0); //CHROME
			break;

		case 4:
			fdiffusecolour = vec4(0.75164, 0.60648, 0.22648, 1.0); //GOLD
			break;

		case 5:
			fdiffusecolour = vec4(0.75164, 0.60648, 0.22648, 1.0); //GOLD
			break;

		case 6:
			fdiffusecolour = vec4(0.7038, 0.27048, 0.191028, 1.0); //COPPER?
			break;

		case 7:
			fdiffusecolour = vec4(0.75164, 0.60648, 0.22648, 1.0); //GOLD
			break;

		case 8:
			fdiffusecolour = vec4(0.40754, 0.40754, 0.40754, 1.0); //SILVER
			break;

		case 9:
			fdiffusecolour = vec4(0.1, 0.1, 0.1, 1.0);
			break;

		case 10:
			fdiffusecolour = vec4(0.3, 0.3, 0.3, 1.0);
			break;

		case 11:
			fdiffusecolour = vec4(color1, color2, color3, 1.0);
			break;

		case 12:
			fdiffusecolour = vec4(0.0, 1.0, 0.0, 1.0);
			break;
		
		case 13:
			fdiffusecolour = vec4(1.0, 0.0, 0.0, 1.0);
			break;

		default:
			fdiffusecolour = vec4(0.9223, 0.9754, 0.9242, 1.0);
			break;
	}

	// Define our vectors for calculating diffuse and specular lighting
	mat4 mv_matrix = view * model;						// Calculate the model-view transformation
	mat3 normalmatrix = mat3(transpose(inverse(mv_matrix)));

	fposition = (mv_matrix * position_h).xyz;			// Modify the vertex position (x, y, z, w) by the model-view transformation
	fnormal = normalize(normalmatrix * normal);	// Modify the normals by the normal-matrix (i.e. to model-view (or eye) coordinates )
	flightdir = light_pos3 - fposition;				// Calculate the vector from the light position to the vertex in eye space

	// Calculate the vertex position in projectin space and output to the pipleline using the reserved variable gl_Position
	gl_Position = (projection * view * model) * position_h;


}
