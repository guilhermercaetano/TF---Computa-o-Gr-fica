#version 130

in vec3 TexCoord;
out vec4 FragColor;
uniform samplerCube CubeTexture;

void main (void) {
    FragColor = texture(CubeTexture, TexCoord);
    //FragColor = vec3(1, 0, 0);
}

//#version 120

//varying vec3 TexCoord;
//uniform samplerCube CubeTexture;
//out vec4 frag_colour;

//void main(void) {
//gl_FragColor = textureCube(CubeTexture, TexCoord);
//}

