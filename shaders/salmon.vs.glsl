#version 330

// Input attributes
in vec3 in_position;
in vec3 in_color; // input attributes are indepenndent for each vertex

out vec3 vcolor;
out vec2 vpos;

// Application data
uniform mat3 transform; // uniform attributes are the same for all vertices
uniform mat3 projection;

void main()
{
	vpos = in_position.xy; // local coordinated before transform
	vcolor = in_color;
//	vec3 pos = projection * transform * vec3(in_position.x, -in_position.y, 1.0); 
	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}