//we will be using glsl version 4.5 syntax
#version 450

layout (location = 0) out vec3 outColor;

void main()
{
	//const array of positions for the triangle
	const vec3 positions[3] = vec3[3](
		vec3(1.f,1.f, 0.0f),
		vec3(-1.f,1.f, 0.0f),
		vec3(0.f,-1.f, 0.0f)
	);

	//const array of colors for the triangle
	const vec3 colors[3] = vec3[3](
		vec3(1.0f, 0.0f, 0.0f), //red
		vec3(0.0f, 1.0f, 0.0f), //green
		vec3(00.f, 0.0f, 1.0f)  //blue
	);

	//gl_VertexIndex is the vulkan glsl version of gl_VertexID, same stuff but different name
	//using glsl opengl extension for syntax highlighting and cant find a vulkan glsl one for my life
	//ignore the extension warning, this is valid vulkan glsl code
	
	//remember this mayy be a cause for issues on intellisence that suggest using the opengl version of parameters
	gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
	outColor = colors[gl_VertexIndex];
}