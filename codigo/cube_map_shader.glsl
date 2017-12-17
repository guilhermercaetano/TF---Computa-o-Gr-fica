#version 130

in vec3 VertexP;
out vec3 TexCoord;
uniform mat4 P, V;

void main() {
    TexCoord = VertexP;
    gl_Position = P * V * vec4(VertexP, 1.0f);
    //gl_Position =  P * V * vec4(VertexP, 1.0f);
}
//#version 120

//in vec3 VertexP;
//uniform mat4 P, V;
//varying vec3 TexCoord;

//void main()
//{
//TexCoord = VertexP;
//gl_Position = P * V * vec4(VertexP, 1.0f);
//}
