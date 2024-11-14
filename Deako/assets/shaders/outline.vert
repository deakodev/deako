#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 4) in uvec4 inJoint0;
layout (location = 5) in vec4 inWeight0;

layout (set = 0, binding = 0) uniform UniformDynamicData
{
	mat4 model;
} uDynamic;

layout (set = 0, binding = 1) uniform UniformSharedData
{
	mat4 projection;
	mat4 view;
	vec3 camPos;
} uShared;

#define MAX_NUM_JOINTS 128

layout (set = 2, binding = 0) uniform UBONode 
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
    float outlineWidth = 0.01f; // Outline width

    vec4 locPos;
    vec3 worldPos;
    vec3 normal;

    if (node.jointCount > 0) 
    {   
        // mesh is skinned
        mat4 skinMatrix = CalculateSkinMatrix();

        locPos = uDynamic.model * node.matrix * skinMatrix * vec4(inPos, 1.0);
        normal = normalize(transpose(inverse(mat3(uDynamic.model * node.matrix * skinMatrix))) * inNormal);
    } 
    else 
    {
        locPos = uDynamic.model * node.matrix * vec4(inPos, 1.0);
        normal = normalize(transpose(inverse(mat3(uDynamic.model * node.matrix))) * inNormal);
    }

    // Apply outline width by offsetting the position along the normal
    locPos.xyz += normal * outlineWidth;

    // Flip Y to match Vulkan convention (optional, depending on coordinate system)
    locPos.y = -locPos.y;

    worldPos = locPos.xyz / locPos.w;

    gl_Position = uShared.projection * uShared.view * vec4(worldPos, 1.0);
}
