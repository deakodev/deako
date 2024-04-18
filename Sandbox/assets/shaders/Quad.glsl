#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexureCoord;
layout(location = 3) in float a_TexureIndex;
layout(location = 4) in float a_TexureScalar;

out vec4 v_Color;
out vec2 v_TexureCoord;
out float v_TexureIndex;
out float v_TexureScalar;

uniform mat4 u_ViewProjection;

void main()
{
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
    v_Color = a_Color;
    v_TexureCoord = a_TexureCoord;
    v_TexureIndex = a_TexureIndex;
    v_TexureScalar = a_TexureScalar;
}

#type fragment
#version 330 core

layout(location = 0) out vec4 fragColor;

in vec4 v_Color;
in vec2 v_TexureCoord;
in float v_TexureIndex;
in float v_TexureScalar;

uniform sampler2D u_Textures[16];

void main()
{
    fragColor = texture(u_Textures[int(v_TexureIndex)], v_TexureCoord * v_TexureScalar) * v_Color;
}
