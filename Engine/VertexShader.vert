#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMat;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

out VS_OUT {
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoords;
} vs_out;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = normalMat * aNormal;
	TexCoords = aTexCoords;
	//gl_PointSize = gl_Position.z;

	vs_out.TexCoords = aTexCoords;
	vs_out.Normal = normalMat * aNormal;
	vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
};