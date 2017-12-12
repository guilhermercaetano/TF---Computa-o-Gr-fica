#version 300 es
layout(location = 0) in vec3 VertexP;

void main(){
    gl_Position.xyz = VertexP;
    gl_Position.w = 1.0;
}

