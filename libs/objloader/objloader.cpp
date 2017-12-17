#include <GL/gl.h>
#include <GL/glut.h>
#include <string>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <string.h>

#include "objloader.h"

using namespace std;

vertec::vertec(){
    X=0;
    Y=0;
    Z=0;
    
    U=0;
    V=0;
    
    nX=0;
    nY=0;
    nZ=0;
}

vertec::vertec(float x, float y, float z , float u, float v, float nx, float ny, float nz){
    X=x;
    Y=y;
    Z=z;
    U=u;
    V=v;
    nX=nx;
    nY=ny;
    nZ=nz;
}

mesh::mesh(){
    
}

//função para carregar o mesh
bool mesh::loadMesh(string path){
    vector<int> VerticeIndex, UVIndex, NormalIndex;//indexadores para vertices, mapeamento de textura e normais
    vector<vertec> tempVert;//vetor temporario de vertices
    vector<vertec> tempTex;// vetor temporario de texturas
    vector<vertec> tempNorm;
    
    FILE* file= fopen(path.data(), "r");
    if(file== NULL){
        cout<< "falha ao carregar o arquivo"<< endl;
        return false;
    }
    else{ 
        while(1){
            char lineHeader[128];
            int res= fscanf(file, "%s", lineHeader);
            if(res==EOF){
                break;
            }
            if(strcmp(lineHeader, "v") == 0){
                float x;
                float y;
                float z;
                fscanf(file, "%f %f %f\n", &x, &y, &z);
                
                float ModelScale = 10.0;
                vertec t(ModelScale*x, ModelScale*y, ModelScale*z,0,0,0,0,0);
                tempVert.push_back(t);
            }
            else if(strcmp(lineHeader,"vt")==0){
                float u;
                float v;
                fscanf(file, "%f %f\n", &u, &v);
                vertec t(0,0,0,u,v,0,0,0);
                tempTex.push_back(t);
            }
            else if(strcmp(lineHeader, "vn")==0){
                float xn;
                float yn;
                float zn;
                fscanf(file, "%f %f %f\n", &xn, &yn, &zn);
                vertec t(0,0,0,0, 0, xn,yn,zn);
                tempNorm.push_back(t);
            }
            else if(strcmp(lineHeader, "f")==0){
                string v1, v2, v3;
                unsigned int vertInd[3], uvInd[3], normInd[3];
                int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertInd[0], &uvInd[0], &normInd[0], &vertInd[1], &uvInd[1], &normInd[1], &vertInd[2], &uvInd[2], &normInd[2]);
                if(matches !=9){
                    printf("QUantidade de valores lidos diferente doesperado. Falha ao ler uma linha de face");
                    return false;			
                }
                //Cria uma lista com os índices na ordem apropriada para o desenho das faces
                //Esta é a lista de índices de vértices
                VerticeIndex.push_back(vertInd[0]);
                VerticeIndex.push_back(vertInd[1]);
                VerticeIndex.push_back(vertInd[2]);
                //Esta é a lista de índices de mapeamento de textura
                UVIndex.push_back(uvInd[0]);
                UVIndex.push_back(uvInd[1]);
                UVIndex.push_back(uvInd[2]);
                // Esta é a lista de índices de normais
                NormalIndex.push_back(normInd[0]);
                NormalIndex.push_back(normInd[1]);
                NormalIndex.push_back(normInd[2]);
            }
        }
        for(unsigned int i=0; i<VerticeIndex.size(); i++){
            vertec tempv(tempVert[VerticeIndex[i]-1].X,tempVert[VerticeIndex[i]-1].Y,tempVert[VerticeIndex[i]-1].Z,tempTex[UVIndex[i]-1].U,tempTex[UVIndex[i]-1].V, tempNorm[NormalIndex[i]-1].nX,tempNorm[NormalIndex[i]-1].nY,tempNorm[NormalIndex[i]-1].nZ);
            verts.push_back(tempv);
        }
        return true;
    }
}


//desenha o mesh
void mesh::draw(){
    glPushMatrix();
    int cont=0;
    float MatEmission[] = { 0.10, 0.10, 0.10, 1};
    float MatColorA[] = { 1, 1, 1, 1};
    float MatColorD[] = { 1.0, 1.0, 1.0, 1};
    float MatSpecular[] = { 0.8, 0.8, 0.8, 1};
    float MatShininess[] = { 50.0 };
    glColor3f(1,1,1);
    
    glMaterialfv(GL_FRONT, GL_EMISSION, MatEmission);
    glMaterialfv(GL_FRONT, GL_AMBIENT, MatColorA);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, MatColorD);
    glMaterialfv(GL_FRONT, GL_SPECULAR, MatSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, MatShininess);
    
    glBindTexture (GL_TEXTURE_2D, meshTex);
    for(unsigned int i=0; i<verts.size(); i=i+3){
        glBegin(GL_POLYGON);
        for(int j=i; j<i+3; j++){
            glNormal3f(verts[j].nX,verts[j].nY,verts[j].nZ);
            glTexCoord2f(verts[j].U,verts[j].V);
            glVertex3f(verts[j].X,verts[j].Y,verts[j].Z);
        }
        glEnd();
    }
    glPopMatrix();
}
