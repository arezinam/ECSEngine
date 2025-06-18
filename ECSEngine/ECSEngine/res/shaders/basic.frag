#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec4 objectColor;
uniform int hasTexture;


void main()
{
    vec4 finalColor;

    if (hasTexture == 1) {
        vec4 texColor = texture(texture_diffuse1, TexCoords);
        finalColor = texColor * objectColor;
    } else {
        finalColor = objectColor;
    }

    
    FragColor = finalColor; 
}