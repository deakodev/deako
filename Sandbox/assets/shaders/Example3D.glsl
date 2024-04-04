// Example 2D Shader

#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;

out vec3 v_Position;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;

void main()
{
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
	v_Position = a_Position;
}

#type fragment
#version 330 core

layout(location = 0) out vec4 fragColor;

in vec3 v_Position;

uniform vec3 u_Color;

void main()
{
	vec3 color = vec3(0.5); // Default color if needed
    float maxComponent = max(max(abs(v_Position.x), abs(v_Position.y)), abs(v_Position.z));

    if(maxComponent == abs(v_Position.x)) {
        color = v_Position.x > 0 ? vec3(1.0, 0.0, 0.0) : vec3(0.5, 0.0, 0.0); // Red shades for +/- X
    } else if(maxComponent == abs(v_Position.y)) {
        color = v_Position.y > 0 ? vec3(0.0, 1.0, 0.0) : vec3(0.0, 0.5, 0.0); // Green shades for +/- Y
    } else if(maxComponent == abs(v_Position.z)) {
        color = v_Position.z > 0 ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 0.0, 0.5); // Blue shades for +/- Z
    }

    fragColor = vec4(color, 1.0) * vec4(u_Color, 1.0);
}
