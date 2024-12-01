//******** PRACTICA VISUALITZACIÓ GRÀFICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn bàsic VS2022 MONOFINESTRA amb OpenGL 4.3, interfície GLFW, ImGui i llibreries GLM
//******** Ferran Poveda, Marc Vivet, Carme Julià, Débora Gil, Enric Martí (Setembre 2024)
// escena.h : interface de escena.cpp
//
#pragma once
#ifndef GLSOURCE_H
#define GLSOURCE_H

// Entorn VGI: OBJECTE OBJ. Include per la definició de l'objecte Obj_OBJ
#include "objLoader.h"	
#include "GameLogic.h"


/* ------------------------------------------------------------------------- */
/*                            Funcions de les escenes                        */
/* ------------------------------------------------------------------------- */
void dibuixa_Eixos(GLuint ax_programID, bool eix, GLuint axis_Id, CMask3D reixa, CPunt3D hreixa, glm::mat4 MatriuProjeccio, glm::mat4 MatriuVista);
void dibuixa_Skybox(GLuint sk_programID, GLuint cmTexture, char eix_Polar, glm::mat4 MatriuProjeccio, glm::mat4 MatriuVista);

// Entorn VGI: dibuixa_EscenaGL -> Dibuix de l'escena GL
void dibuixa_EscenaGL(
    GLuint sh_programID,
    CColor col_object,
    bool sw_mat[5],
    bool textur,
    GLuint texturID[NUM_MAX_TEXTURES],
    bool textur_map,
    bool flagInvertY,
    COBJModel* objecteOBJ,
    glm::mat4 MatriuVista,
    glm::mat4 MatriuTG,
    GameLogic& game,
    bool& garage,
    int& actCar,
    float time
);

void garageDraw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, int& actCar, float logicTime);

// ALBERT
void dibuixaCotxe(GLuint sh_programID, bool sw_mat[5], glm::mat4 MatriuVista,
	glm::mat4 MatriuTG, COBJModel* objecteOBJ, float x, float y, float z, float angle);


// Entorn VGI: dibuixa -> Funció que dibuixa objectes simples de la llibreria GLUT segons obj
void dibuixa(GLuint sh_programID, char obj, glm::mat4 MatriuVista, glm::mat4 MatriuTG);

// Entorn VGI: Objecte Arc
void arc(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5]);
//oid sea(GLint shaderId, glm::mat4 MatriuVista, glm::mat4 MatriuTG, CColor colorM);
CVAO loadSea_VAO(CColor colorM);

// Entorn VGI: Tie (Nau enemiga Star Wars) - Objecte fet per alumnes.
void tie(GLint shaderId, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[4]);
void Alas(GLint shaderId, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[4]);
void Motor(GLint shaderId, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[4]);
void Canon(GLint shaderId, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[4]);
void Cuerpo(GLint shaderId, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[4]);
void Cabina(GLint shaderId, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[4]);

// Entorn VGI: Funcions locals a escena.cpp
// CString2char: Funció de conversió de variable CString a char * per a noms de fitxers 
//char* CString2Char(CString entrada);

static float rotationAngle = 0;

#endif