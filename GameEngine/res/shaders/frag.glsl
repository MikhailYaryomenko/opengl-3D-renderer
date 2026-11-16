#version 330 core

in vec2 v_TexCoord;

uniform sampler2D u_Texture;

out vec4 color;

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoord);
    if (texColor.a < 0.05)
        discard;
    color = texColor;
}
