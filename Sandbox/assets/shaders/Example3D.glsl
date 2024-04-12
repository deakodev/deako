// Basic texture shader

#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection; 

void main()
{
    gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
    v_TexCoord = a_TexCoord;
}

#type fragment
#version 330 core

layout(location = 0) out vec4 fragColor;

in vec2 v_TexCoord;

uniform vec4 u_Color;
uniform float u_TexScalar;
uniform sampler2D u_Texture;

void main()
{
    fragColor = texture(u_Texture, v_TexCoord * u_TexScalar) * u_Color;
}
