#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec2 a_TexureCoord;
layout(location = 4) in float a_TexureIndex;
layout(location = 5) in float a_TexureScalar;

out vec3 v_FragPosition;
out vec4 v_Color;
out vec2 v_TexureCoord;
out float v_TexureIndex;
out float v_TexureScalar;
out vec3 v_Normal;

uniform mat4 u_ViewProjection;

void main()
{
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
    v_FragPosition = a_Position;
    v_Color = a_Color;
    v_TexureCoord = a_TexureCoord;
    v_TexureIndex = a_TexureIndex;
    v_TexureScalar = a_TexureScalar;
    v_Normal = a_Normal;
}

#type fragment
#version 330 core

layout(location = 0) out vec4 fragColor;

in vec3 v_FragPosition;
in vec4 v_Color;
in vec2 v_TexureCoord;
in float v_TexureIndex;
in float v_TexureScalar;
in vec3 v_Normal;

uniform sampler2D u_Textures[16];
uniform vec3 u_LightPosition;
uniform vec3 u_ViewPosition;

void main()
{
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    vec3 normal = normalize(v_Normal);
    vec3 lightDirection = normalize(u_LightPosition - v_FragPosition);
    
    float diffuseImpact = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = diffuseImpact * lightColor;

    float specularStrength = 0.1;
    vec3 viewDirection = normalize(u_ViewPosition - v_FragPosition);
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec4 lightResult = vec4(ambient + diffuse + specular, 1.0) * v_Color;

    fragColor = texture(u_Textures[int(v_TexureIndex)], v_TexureCoord * v_TexureScalar) * lightResult;
}
