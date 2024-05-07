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

    vec4 textureColor = lightResult;
	switch(int(v_TexureIndex))
	{
		case 0: textureColor *= texture(u_Textures[0], v_TexureCoord * v_TexureScalar); break;
		case 1: textureColor *= texture(u_Textures[1], v_TexureCoord * v_TexureScalar); break;
		case 2: textureColor *= texture(u_Textures[2], v_TexureCoord * v_TexureScalar); break;
		case 3: textureColor *= texture(u_Textures[3], v_TexureCoord * v_TexureScalar); break;
		case 5: textureColor *= texture(u_Textures[5], v_TexureCoord * v_TexureScalar); break;
		case 6: textureColor *= texture(u_Textures[6], v_TexureCoord * v_TexureScalar); break;
		case 7: textureColor *= texture(u_Textures[7], v_TexureCoord * v_TexureScalar); break;
		case 8: textureColor *= texture(u_Textures[8], v_TexureCoord * v_TexureScalar); break;
		case 9: textureColor *= texture(u_Textures[9], v_TexureCoord * v_TexureScalar); break;
		case 10: textureColor *= texture(u_Textures[10], v_TexureCoord * v_TexureScalar); break;
		case 11: textureColor *= texture(u_Textures[11], v_TexureCoord * v_TexureScalar); break;
		case 12: textureColor *= texture(u_Textures[12], v_TexureCoord * v_TexureScalar); break;
		case 13: textureColor *= texture(u_Textures[13], v_TexureCoord * v_TexureScalar); break;
		case 14: textureColor *= texture(u_Textures[14], v_TexureCoord * v_TexureScalar); break;
		case 15: textureColor *= texture(u_Textures[15], v_TexureCoord * v_TexureScalar); break;
	}
	fragColor = textureColor;
}
