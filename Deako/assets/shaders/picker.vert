#version 450 

layout (location = 0) in vec3 inPos;
layout (location = 1) in uvec4 inJoint0;
layout (location = 2) in vec4 inWeight0;

layout (set = 0, binding = 1) uniform UniformDataDynamic
{
	mat4 model;
} uDynamic;

layout (set = 0, binding = 2) uniform UniformDataShared
{
	mat4 projection;
	mat4 view;
	vec3 camPos;
} uShared;

#define MAX_NUM_JOINTS 128

layout (set = 1, binding = 0) uniform UBONode 
{
	mat4 matrix;
	mat4 jointMatrix[MAX_NUM_JOINTS];
	uint jointCount;
} node;

mat4 CalculateSkinMatrix()
{
	return inWeight0.x * node.jointMatrix[inJoint0.x] +
		inWeight0.y * node.jointMatrix[inJoint0.y] +
		inWeight0.z * node.jointMatrix[inJoint0.z] +
		inWeight0.w * node.jointMatrix[inJoint0.w];
}


void main()
{
	vec4 locPos;

	if (node.jointCount > 0) 
	{	// mesh is skinned
		mat4 skinMatrix = CalculateSkinMatrix();
		locPos = uDynamic.model * node.matrix * skinMatrix * vec4(inPos, 1.0);
	} 
	else 
	{
		locPos = uDynamic.model * node.matrix * vec4(inPos, 1.0);
	}

	locPos.y = -locPos.y;
	vec3 worldPos = locPos.xyz / locPos.w;

	gl_Position = uShared.projection * uShared.view * vec4(worldPos, 1.0);
} 
