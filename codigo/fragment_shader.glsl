#version 300 es

out highp vec3 DiffuseColor;
out int MaterialId;
out highp vec4 SpecularColor;
out highp vec3 Position;
out highp vec3 Normal;

void main(){
    DiffuseColor = vec3(1,0,0);
    //SpecularColor = vec4(1,1,1,0.5);
    //Normal = vec3(0, 0, 1);
}

