//******** PRACTICA VISUALITZACI� GR�FICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn b�sic VS2022 MONOFINESTRA amb OpenGL 4.3, interf�cie GLFW, ImGui i llibreries GLM
//******** Ferran Poveda, Marc Vivet, Carme Juli�, D�bora Gil, Enric Mart� (Setembre 2024)
// escena.cpp : Aqui es on ha d'anar el codi de les funcions que 
//              dibuixin les escenes.
//
//    Versi� 2.0:	- Objectes Cub, Esfera, Tetera (primitives libreria GLUT)
//
//	  Versi� 2.2:	- Objectes Cub, Esfera, Tetera definides en fitxer font glut_geometry amb altres primitives GLUT
//
//	  Versi� 2.5:	- Objectes cubRGB i Tie (nau Star Wars fet per alumnes)
//

#include "stdafx.h"
#include "material.h"
#include "visualitzacio.h"
#include "escena.h"

// Dibuixa Eixos Coordenades M�n i Reixes, activant un shader propi.
void dibuixa_Eixos(GLuint ax_programID, bool eix, GLuint axis_Id, CMask3D reixa, CPunt3D hreixa, 
	glm::mat4 MatriuProjeccio, glm::mat4 MatriuVista)
{
// Visualitzaci� Eixos Coordenades M�n
	glUseProgram(ax_programID);

// Pas Matrius Projecci� i Vista Vista a shader
	glUniformMatrix4fv(glGetUniformLocation(ax_programID, "projectionMatrix"), 1, GL_FALSE, &MatriuProjeccio[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(ax_programID, "viewMatrix"), 1, GL_FALSE, &MatriuVista[0][0]);

// Attribute Locations must be setup before calling glLinkProgram()
	glBindAttribLocation(ax_programID, 0, "in_Vertex"); // V�rtexs
	glBindAttribLocation(ax_programID, 1, "in_Color");	// Color

//  Dibuix dels eixos
	if (eix) draw_Eixos(axis_Id);

// Dibuixa el grid actiu
	if ((reixa.x) || (reixa.y) || (reixa.z) || (reixa.w)) draw_Grid(reixa, hreixa);
}

// Dibuixa Skybox en forma de Cub, activant un shader propi.
void dibuixa_Skybox(GLuint sk_programID, GLuint cmTexture, char eix_Polar, glm::mat4 MatriuProjeccio, glm::mat4 MatriuVista)
{
	glm::mat4 ModelMatrix(1.0);

	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content

// Activaci� shader per a cub skybox
	glUseProgram(sk_programID);

// Pas Matrius Projecci� i Vista a shader
	glUniformMatrix4fv(glGetUniformLocation(sk_programID, "projectionMatrix"), 1, GL_FALSE, &MatriuProjeccio[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(sk_programID, "viewMatrix"), 1, GL_FALSE, &MatriuVista[0][0]);

// Rotar skyBox per a orientar sobre eix superior Z o X en Vista Esf�rica (POLARX, POLARY, POLARZ)
	if (eix_Polar == POLARZ) ModelMatrix = glm::rotate(ModelMatrix, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
	else if (eix_Polar == POLARX) ModelMatrix = glm::rotate(ModelMatrix, radians(-90.0f), vec3(0.0f, 0.0f, 1.0f));

// Escalar Cub Skybox a 5000 per encabir objectes escena a l'interior
	ModelMatrix = glm::scale(ModelMatrix, vec3(5000.0f, 5000.0f, 5000.0f));		//glScaled(5000.0, 5000.0, 5000.0);
	glUniformMatrix4fv(glGetUniformLocation(sk_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);

// Activar textura cubemaps del Skybox per encabir objectes escena
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cmTexture);

// Attribute Locations must be setup before calling glLinkProgram()
	glBindAttribLocation(sk_programID, 0, "in_Vertex"); // V�rtexs

//  Dibuix del Skybox
	drawCubeSkybox();

	glDepthFunc(GL_LESS); // set depth function back to default
}



// dibuixa_EscenaGL: Dibuix de l'escena amb comandes GL
void dibuixa_EscenaGL(GLuint sh_programID, CColor col_object, bool sw_mat[5], bool textur, GLuint texturID[NUM_MAX_TEXTURES],
	bool textur_map, bool flagInvertY, COBJModel* objecteOBJ,	glm::mat4 MatriuVista, glm::mat4 MatriuTG, GameLogic& game, bool& garage, int& actCar, float delta)
{
	float altfar = 0;
	GLint npunts = 0, nvertexs = 0;
	int i, j, k;
	GLdouble tras[3] = { 0.0,0.0,0.0 }; //Sierpinski Sponge
	CColor color_vermell = { 0.0,0.0,0.0,1.0 }, color_Mar = { 0.0,0.0,0.0,0.0 };
	bool sw_material[5] = { 0.0,0.0,0.0,0.0,0.0 };
	
// Matrius de Transformaci�
	glm::mat4 NormalMatrix(1.0), ModelMatrix(1.0), TransMatrix(1.0), ScaleMatrix(1.0), RotMatrix(1.0);

// VAO
	CVAO objectVAO = { 0,0,0,0,0 };
	objectVAO.vaoId = 0;	objectVAO.vboId = 0;	objectVAO.eboId = 0;	 objectVAO.nVertexs = 0; objectVAO.nIndices = 0;

	tras[0] = 0.0;	tras[1] = 0.0; tras[2] = 0.0;
	color_vermell.r = 1.0;	color_vermell.g = 0.0; color_vermell.b = 0.0; color_vermell.a = 1.0;
	sw_material[0] = false;	sw_material[1] = true; sw_material[2] = true; sw_material[3] = false;	sw_material[4] = true;

// Shader Visualitzaci� Objectes
	glUseProgram(sh_programID);

// Parametritzaci� i activaci�/desactivaci� de textures
	if (texturID[0] != -1) SetTextureParameters(texturID[0], true, true, textur_map, false);
	if (textur) {	glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_TRUE); //glEnable(GL_TEXTURE_2D);
					glUniform1i(glGetUniformLocation(sh_programID, "modulate"), GL_TRUE); //glEnable(GL_MODULATE);
				}
		else {	glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE); //glDisable(GL_TEXTURE_2D);
				glUniform1i(glGetUniformLocation(sh_programID, "modulate"), GL_FALSE); //glDisable(GL_MODULATE);
			}
	glUniform1i(glGetUniformLocation(sh_programID, "flag_invert_y"), flagInvertY);

// Attribute Locations must be setup before calling glLinkProgram()
	glBindAttribLocation(sh_programID, 0, "in_Vertex");		// V�rtexs
	glBindAttribLocation(sh_programID, 1, "in_Color");		// Color
	glBindAttribLocation(sh_programID, 2, "in_Normal");		// Normals
	glBindAttribLocation(sh_programID, 3, "in_TexCoord");	// Textura

// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
	SeleccionaColorMaterial(sh_programID, col_object, sw_mat);
	//game.road.setRoadSize(50, 1000); LEVON mirar on posar les dimensions de la carretera
	/*game.road.draw(sh_programID, MatriuVista, MatriuTG);
	game.player.draw(sh_programID, MatriuVista, MatriuTG);*/ // ALBERT de moment nom�s dibuixo el jugador, cal mirar coordenades i tal per a dibuixar tot b�

	game.player.m_model = CAR_MODELS[actCar];
	game.player.m_height = (CAR_WIDTH / game.player.m_model->m_width) * game.player.m_model->m_depth;
	for (int i = 0; i < 3; i++)
	{
		game.player.m_collisionCircles[i] = Circle(game.player.m_x, game.player.m_y - game.player.m_height / 4 + game.player.m_height / 4 * i, game.player.m_width / 2);
	}

	if (!garage)
	{
		game.draw(sh_programID, MatriuVista, MatriuTG);
	}
	else
		garageDraw(sh_programID, MatriuVista, MatriuTG, actCar, delta);
		

// Enviar les comandes gr�fiques a pantalla
//	glFlush();
}

float Ptime = 0.0f;

void garageDraw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, int& actCar, float delta)
{
	Car* c = new Car();
	c->assign(CAR_MODELS[actCar]);
	c->m_x = 0;
	c->m_y = 0;
	
	Ptime += delta;
	if (Ptime > FRAME_TIME)
	{
		rotationAngle += 0.5;
		if (rotationAngle > 360.0f)
			rotationAngle -= 360.0f;
		Ptime -= FRAME_TIME;
	}

	float amplitude = 10.0f;
	float frequency = 2.0f;

	float deltaY = amplitude * sin(glm::radians(rotationAngle) * frequency);

	MatriuTG = glm::mat4(1.0f);
	MatriuTG = glm::translate(MatriuTG, glm::vec3(0.0f, deltaY, 0.0f));
	MatriuTG = glm::rotate(MatriuTG, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	c->draw(sh_programID, MatriuVista, MatriuTG);
}

// dibuixa: Funci� que dibuixa objectes simples de la llibreria GLUT segons obj
void dibuixa(GLuint sh_programID, char obj, glm::mat4 MatriuVista, glm::mat4 MatriuTG)
{
//	std::string String;
//	const char* string;
	glm::mat4 NormalMatrix(1.0), ModelMatrix(1.0); // TransMatrix(1.0); // ScaleMatrix(1.0), RotMatrix(1.0);
	GLdouble tras[3] = { 0.0,0.0,0.0 }; //Sierpinski Sponge
	
	tras[0] = 0.0;	tras[1] = 0.0; tras[2] = 0.0;

	switch(obj)
	{

// Cub
	case CUB:
		//glPushMatrix();
		  //glScaled(5.0,5.0,5.0);
		ModelMatrix = glm::scale(MatriuTG, vec3(5.0f, 5.0f, 5.0f));
		// Pas ModelView Matrix a shader
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		// Pas NormalMatrix a shader
		NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
		draw_TriEBO_Object(GLUT_CUBE);	//draw_TriVAO_Object(GLUT_CUBE);  //glutSolidCube(1.0);
		//glPopMatrix();

		break;

	case CUB_RGB:
		//glPushMatrix();
		  //glScaled(5.0,5.0,5.0);
		ModelMatrix = glm::scale(MatriuTG, vec3(5.0f, 5.0f, 5.0f));
		// Pas ModelView Matrix a shader
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		// Pas NormalMatrix a shader
		NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
		draw_TriEBO_Object(GLUT_CUBE_RGB);	//draw_TriVAO_Object(GLUT_CUBE_RGB); // glutSolidCubeRGB(1.0);
		//glPopMatrix();
		break;

// Esfera
	case ESFERA:	
		//glPushMatrix();
		  //glScaled(5.0,5.0,5.0);
		ModelMatrix = glm::scale(MatriuTG, vec3(5.0f, 5.0f, 5.0f));
		// Pas ModelView Matrix a shader
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		// Pas NormalMatrix a shader
		NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
		draw_TriEBO_Object(GLU_SPHERE); //draw_TriVAO_Object(GLU_SPHERE); //gluSphere(1.0, 40, 40);
		//glPopMatrix();
		break;

// Tetera
	case TETERA:
		//glPushMatrix();
		  //glScaled(5.0,5.0,5.0);
		  ModelMatrix = MatriuTG;
		  // Pas ModelView Matrix a shader
		  glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		  // Pas NormalMatrix a shader
		  NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
		  glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
		  draw_TriVAO_Object(GLUT_TEAPOT); // glutSolidTeapot(1.0);
		//glPopMatrix();
		break;
	}
}

// OBJECTE ARC
void arc(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[5])
{	CColor col_object = { 0.0,0.0,0.0,1.0 };
	glm::mat4 NormalMatrix(1.0), ModelMatrix(1.0);

// Definir VAO del Cub
	//glutSolidCube_VAO(1.0);

// Pota Esquerra
	//glPushMatrix();
	  //glTranslated(0.0, 0.0, 2.5);
	  //glScaled(1.0, 1.0, 5.0);
	ModelMatrix = glm::translate(MatriuTG, vec3(0.0f, 0.0f, 2.5f));
	ModelMatrix = glm::scale(ModelMatrix, vec3(1.0f, 1.0f, 5.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	// Pas NormalMatrix a shader
	NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	draw_TriEBO_Object(GLUT_CUBE); // drawSolidCube();	//  glutSolidCube(1.0);
	//glPopMatrix();

// Pota Dreta
	//glPushMatrix();
	//glTranslated(0.0, 5.0, 2.5);
	//glScaled(1.0, 1.0, 5.0);
	ModelMatrix = glm::translate(MatriuTG, vec3(0.0f, 5.0f, 2.5f));
	ModelMatrix = glm::scale(ModelMatrix, vec3(1.0f, 1.0f, 5.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	// Pas NormalMatrix a shader
	NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	draw_TriEBO_Object(GLUT_CUBE); //drawSolidCube();	//  glutSolidCube(1.0);
	//glPopMatrix();

// Travesser
	//glPushMatrix();
	  //glTranslated(0.0, 2.5, 5.5);
	  //glRotated(90.0, 1.0, 0.0, 0.0);
	  //glScaled(1.0, 1.0, 6.0);
	ModelMatrix = glm::translate(MatriuTG, vec3(0.0f, 2.5f, 5.5f));
	ModelMatrix = glm::rotate(ModelMatrix, radians(90.f),vec3(1.0f, 0.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, vec3(1.0f, 1.0f, 6.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	// Pas NormalMatrix a shader
	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	draw_TriEBO_Object(GLUT_CUBE);  //drawSolidCube();	//  glutSolidCube(1.0);
	//glPopMatrix();

// Eliminar VAO del Cub
	//deleteVAO(GLUT_CUBE);

// Esfera
	col_object.r = 0.0;		col_object.g = 1.0;		col_object.b = 1.0;		col_object.a = 1.0;	// Color blau clar
	SeleccionaColorMaterial(sh_programID, col_object, sw_mat);
	//glPushMatrix();
	  //glTranslated(0.0, 5.0, 6.5);
    ModelMatrix = glm::translate(MatriuTG, vec3(0.0f, 5.0f, 6.5f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	// Pas NormalMatrix a shader
	NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	draw_TriEBO_Object(GLU_SPHERE); //gluSphere(0.5, 20, 20);
	//glPopMatrix();

// Tetera
	col_object.r = 1.0;		col_object.g = 1.0;		col_object.b = 0.0;		col_object.a = 1.0;	// Color groc
	SeleccionaColorMaterial(sh_programID, col_object, sw_mat);
	//glPushMatrix();
	  //glTranslated(0.0, 0.0, 6.0);
	  //glRotated(-90.0, 0.0, 0.0, 1.0);
	  //glRotated(90.0, 1.0, 0.0, 0.0);
	  //glScaled(0.25, 0.25, 0.25);
	ModelMatrix = glm::translate(MatriuTG, vec3(0.0f, 0.0f, 6.0f));
	ModelMatrix = glm::rotate(ModelMatrix, radians(-90.f), vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::rotate(ModelMatrix, radians(90.f), vec3(1.0f, 0.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, vec3(0.25f, 0.25f, 0.25f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	// Pas NormalMatrix a shader
	NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	draw_TriVAO_Object(GLUT_TEAPOT); //glutSolidTeapot(1.0);
	//glPopMatrix();
}


// Mar amb ondulacions
CVAO loadSea_VAO(CColor colorM)
{
	int i, j, step, it1, it2;
	double angle, delay, Nx;
	glm::mat4 ModelMatrix(1.0), NormalMatrix(1.0);

// VAO
	GLuint vaoId = 0; GLuint vboId = 0;
	CVAO seaVAO = { 0,0,0,0,0 };
	seaVAO.vaoId = 0;	seaVAO.vboId = 0;	seaVAO.nVertexs = 0;	seaVAO.eboId = 0; seaVAO.nIndices = 0;
	std::vector <double> vertices, normals, colors, textures;	// Definici� vectors din�mics per a vertexs, normals i textures 
	vertices.resize(0);		normals.resize(0);		colors.resize(0);	textures.resize(0);			// Reinicialitzar vectors

// Aigua amb ondulacions simulades a partir de normals sinusoidals
	step = 10; //step = 5;
	delay = 0;
	it1 = 0;

//   SeleccionaMaterial(MAT_DEFAULT);	
	float h = 2 * PI*step / 500;
	for (j = -250; j<250 - step; j = j + step)
	{	delay = 1.0*h*it1;

		it2 = 0;
		//glColor3f(0.5f, 0.4f, 0.9f);
		for (i = -250; i<250 - step; i = i + step)
		{	//glBegin(GL_POLYGON);
			  angle = 1.0*it2*h * 15;
			  Nx = -cos(angle);
			  //glNormal3f(-cos(angle), 0, 1);
			  //glVertex3f(i, j, 0);			// V�rtex P1
			  colors.push_back(colorM.r);		colors.push_back(colorM.g);		colors.push_back(colorM.b);		colors.push_back(colorM.a);  // Vector Colors
			  normals.push_back(Nx);			normals.push_back(0.0);			normals.push_back(1.0);			// Vector Normals
			  textures.push_back(0.0);			textures.push_back(0.0);										// Vector Textures
			  vertices.push_back(i);			vertices.push_back(j);			vertices.push_back(0.0);		// Vector Vertices

			  angle = 1.0*(it2 + 1.0)*h * 15;
			  Nx = -cos(angle);
			  //glNormal3f(-cos(angle), 0, 1);
			  //glVertex3f(i + step, j, 0);		// V�rtex P2
			  colors.push_back(colorM.r);		colors.push_back(colorM.g);		colors.push_back(colorM.b);		colors.push_back(colorM.a);  // Vector Colors
			  normals.push_back(Nx);			normals.push_back(0.0);			normals.push_back(1.0);			// Vector Normals
			  textures.push_back(1.0);			textures.push_back(0.0);										// Vector Textures
			  vertices.push_back(i+step);		vertices.push_back(j);			vertices.push_back(0.0);		// Vector Vertices

			  angle = 1.0*(it2 + 1.0)*h * 15;
			  Nx = -cos(angle);
			  //glNormal3f(-cos(angle), 0, 1);
			  //glVertex3f(i + step, j + step, 0);// V�rtex P3
			  colors.push_back(colorM.r);		colors.push_back(colorM.g);		colors.push_back(colorM.b);		colors.push_back(colorM.a);  // Vector Colors
			  normals.push_back(Nx);			normals.push_back(0.0);			normals.push_back(1.0);			// Vector Normals
			  textures.push_back(1.0);			textures.push_back(1.0);										// Vector Textures
			  vertices.push_back(i+step);		vertices.push_back(j+step);		vertices.push_back(0.0);		// Vector Vertices

			//glEnd();
// ----------------------- VAO
			//std::vector <int>::size_type nv = vertices.size();	// Tamany del vector vertices en elements.
			//draw_GL_TRIANGLES_VAO(vertices, normals, colors, textures);
			//vertices.resize(0);	normals.resize(0);	colors.resize(0);	textures.resize(0);	// Reinicialitzar vectors

			//glBegin(GL_POLYGON);
			  angle = 1.0*it2*h * 15;
			  //glNormal3f(-cos(angle), 0, 1);
			  //glVertex3f(i, j, 0);			// V�rtex P1
			  colors.push_back(colorM.r);		colors.push_back(colorM.g);		colors.push_back(colorM.b);		colors.push_back(colorM.a);  // Vector Colors
			  normals.push_back(-cos(angle));	normals.push_back(0.0);			normals.push_back(1.0);			// Vector Normals
			  textures.push_back(1.0);			textures.push_back(1.0);										// Vector Textures
			  vertices.push_back(i);			vertices.push_back(j);			vertices.push_back(0.0);		// Vector Vertices

			  angle = 1.0*(it2 + 1)*h * 15;
			  //glNormal3f(-cos(angle), 0, 1);
			  //glVertex3f(i + step, j + step, 0);// V�rtex P2
			  colors.push_back(colorM.r);		colors.push_back(colorM.g);		colors.push_back(colorM.b);		colors.push_back(colorM.a);  // Vector Colors
			  normals.push_back(-cos(angle));	normals.push_back(0.0);			normals.push_back(1.0);			// Vector Normals
			  textures.push_back(1.0);			textures.push_back(1.0);										// Vector Textures
			  vertices.push_back(i + step);		vertices.push_back(j + step);	vertices.push_back(0.0);		// Vector Vertices

			  angle = 1.0*it2*h * 15;
			  //glNormal3f(-cos(angle), 0, 1);
			  //glVertex3f(i, j + step, 0);		// V�rtex P3
			  colors.push_back(colorM.r);		colors.push_back(colorM.g);		colors.push_back(colorM.b);		colors.push_back(colorM.a);  // Vector Colors
			  normals.push_back(-cos(angle));	normals.push_back(0.0);			normals.push_back(1.0);			// Vector Normals
			  textures.push_back(0.0);			textures.push_back(1.0);										// Vector Textures
			  vertices.push_back(i);			vertices.push_back(j + step);		vertices.push_back(0.0);	// Vector Vertices

			//glEnd();

			it2++;
		}
		it1++;
	}
// ----------------------- VAO
	std::vector <int>::size_type nv = vertices.size();	// Tamany del vector vertices en elements.
	//draw_GL_TRIANGLES_VAO(vertices, normals, colors, textures);
	// 
// Creaci� d'un VAO i un VBO i c�rrega de la geometria. Guardar identificador VAO identificador VBO a struct CVAO.
	seaVAO = load_TRIANGLES_VAO(vertices, normals, colors, textures);
	//seaVAO.vaoId = vaoId;
	//seaVAO.vboId = vboId;

	seaVAO.nVertexs = (int)nv / 3;
	return seaVAO;
}


// OBJECTE TIE: FETS PER ALUMNES -----------------------------------------------------------------

// Objecte TIE
void tie(GLint shaderId, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[4])
{	Motor(shaderId,MatriuVista,MatriuTG, sw_mat);
	Alas(shaderId, MatriuVista, MatriuTG, sw_mat);
	Canon(shaderId, MatriuVista, MatriuTG, sw_mat);
	Cuerpo(shaderId, MatriuVista, MatriuTG, sw_mat);
	Cabina(shaderId, MatriuVista, MatriuTG, sw_mat);
};

void Alas(GLint shaderId, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[4])
{
// Matrius Transformaci�
	glm::mat4 TransMatrix(1.0), ModelMatrix(1.0), NormalMatrix(1.0);
	CColor col_object = { 0.0,0.0,0.0,1.0 };
	
	draw_TriEBO_Object(GLU_DISK); //int nvertD = gluDisk_VAO(0.0f, 5.0f, 6, 1);

// Ala 1 Parte central
//Parte Exterior
	//glPushMatrix();
	  //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
	  col_object.r = 1.0;	col_object.g = 1.0;		col_object.b = 1.0;	 col_object.a = 1.0;
	  SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	  //glTranslatef(25.25f, 0.0f, 0.0f);
	  //glRotated(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScaled(10.0f, 10.0f, 5.0f);
	  TransMatrix = glm::translate(MatriuTG, vec3(25.25f, 0.0f, 0.0f));
	  TransMatrix = glm::rotate(TransMatrix,radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	  ModelMatrix = glm::scale(TransMatrix, vec3(10.0f, 10.0f, 5.0f));
	  // Pas ModelView Matrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	  NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	  // Pas NormalMatrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	  
	  draw_TriEBO_Object(GLU_CYLINDER);	//gluCylinder(5.0f, 5.0f, 0.5f, 6, 1);
	//glPopMatrix();

	//Tapas
	//glPushMatrix();
//	  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	  //glTranslatef(25.25f, 0.0f, 0.0f);
	  //glRotated(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScaled(10.0f, 10.0f, 5.0f);
	  TransMatrix = glm::translate(MatriuTG, vec3(25.25f, 0.0f, 0.0f));
	  TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	  ModelMatrix = glm::scale(TransMatrix, vec3(10.0f, 10.0f, 5.0f));
	  // Pas ModelView Matrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	  NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	  // Pas NormalMatrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	  //gluDisk(0.0f, 5.0f, 6, 1);
	  draw_TriEBO_Object(GLU_DISK); // drawgluDisk(nvertD);
	//glPopMatrix();

	//glPushMatrix();
//	  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	  //glTranslatef(27.75f, 0.0f, 0.0f);
	  //glRotated(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScaled(10.0f, 10.0f, 5.0f);
	  TransMatrix = glm::translate(MatriuTG, vec3(27.75f, 0.0f, 0.0f));
	  TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	  ModelMatrix = glm::scale(TransMatrix, vec3(10.0f, 10.0f, 5.0f));
	  // Pas ModelView Matrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	  NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	  // Pas NormalMatrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	  //gluDisk(0.0f, 5.0f, 6, 1);
	  draw_TriEBO_Object(GLU_DISK); // drawgluDisk(nvertD);
	//glPopMatrix();

//Parte Interior
	//glPushMatrix();
	  //glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	  SetColor4d(0.0, 0.0, 0.0, 1.0);
// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
	  col_object.r = 0.0;	col_object.g = 0.0;		col_object.b = 0.0;	 col_object.a = 1.0;
	  SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	  //glTranslated(25.25f, 0.0f, 0.0f);
	  //glRotated(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScaled(2.0f, 2.0f, 2.0f);
	  TransMatrix = glm::translate(MatriuTG, vec3(25.25f, 0.0f, 0.0f));
	  TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	  ModelMatrix = glm::scale(TransMatrix, vec3(2.0f, 2.0f, 2.0f));
	  // Pas ModelView Matrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	  NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	  // Pas NormalMatrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	  draw_TriEBO_Object(GLUT_USER1); // gluCylinder(5.0f, 5.0f, 2.0f, 6, 1);
	//glPopMatrix();

//Tapas
	//glPushMatrix();
	  //glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	  //glTranslatef(25.25f, 0.0f, 0.0f);
	  //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScalef(2.0f, 2.0f, 2.0f);
	  TransMatrix = glm::translate(MatriuTG, vec3(25.75f, 0.0f, 0.0f));
	  TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	  ModelMatrix = glm::scale(TransMatrix, vec3(2.0f, 2.0f, 2.0f));
	  // Pas ModelView Matrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	  NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	  // Pas NormalMatrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	  //gluDisk(0.0f, 5.0f, 6, 1);
	  draw_TriEBO_Object(GLU_DISK); //drawgluDisk(nvertD);
	//glPopMatrix();

	//glPushMatrix();
	  //glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	  //glTranslatef(29.25f, 0.0f, 0.0f);
	  //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScalef(2.0f, 2.0f, 2.0f);
	  TransMatrix = glm::translate(MatriuTG, vec3(29.25f, 0.0f, 0.0f));
	  TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	  ModelMatrix = glm::scale(TransMatrix, vec3(2.0f, 2.0f, 2.0f));
	  // Pas ModelView Matrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	  NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	  // Pas NormalMatrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	  //gluDisk(0.0f, 5.0f, 6, 1);
	  draw_TriEBO_Object(GLU_DISK); //drawgluDisk(nvertD);
	//glPopMatrix();

//Decoracion Exterior
	int j;
	//glPushMatrix();
	  //glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	  //glTranslatef(26.5f, 0.0f, 0.0f);
	  //glScalef(15.75f, 13.75f, 13.75f);
	  //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	  //glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
	  TransMatrix = glm::translate(MatriuTG, vec3(26.5f, 0.0f, 0.0f));
	  TransMatrix = glm::scale(TransMatrix, vec3(15.75f, 13.75f, 13.75f));
	  TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	  TransMatrix = glm::rotate(TransMatrix, radians(30.f), vec3(0.0f, 0.0f, 1.0f));
	  for (j = 0; j<3; j = j + 1)
		{	//glRotatef(60, 0, 0, 1);
			TransMatrix = glm::rotate(TransMatrix, radians(60.f), vec3(0.0f, 0.0f, 1.0f));
			//glPushMatrix();
			   //glScalef(12.7f * 0.583f, 0.1f, 0.3f);
				ModelMatrix = glm::scale(TransMatrix, vec3(12.7f * 0.583f, 0.1f, 0.3f));
				// Pas ModelView Matrix a shader
				glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
				NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
				// Pas NormalMatrix a shader
				glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
				draw_TriEBO_Object(GLUT_CUBE); //glutSolidCube(1.0f);
			//glPopMatrix();
		}
	//glPopMatrix();

	int u;
	//glPushMatrix();
	//glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	TransMatrix = MatriuTG;
	for (u = 0; u < 6; u = u + 1)
	{	//glRotated(60, 1.0, 0.0, 0.0);
		TransMatrix = glm::rotate(TransMatrix, radians(60.f), vec3(1.0f, 0.0f, 0.0f));
		//glPushMatrix();
			//glTranslatef(26.5f, 0.0f, 43.25f);
			//glRotatef(90, 0.0, 0.0, 1.0);
			//glScalef(10 * 5.155f, 5.0f, 3.0f);
			ModelMatrix = glm::translate(TransMatrix, vec3(26.5f, 0.0f, 43.25f));
			ModelMatrix = glm::rotate(ModelMatrix, radians(90.f), vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::scale(ModelMatrix, vec3(10 * 5.155f, 5.0f, 3.0f));
			// Pas ModelView Matrix a shader
			glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
			// Pas NormalMatrix a shader
			glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
			draw_TriEBO_Object(GLUT_CUBE); //glutSolidCube(1.0f);
		//glPopMatrix();
	}
	//glPopMatrix();


//Ala 2 Parte central 
//Parte Exterior
	//glPushMatrix();
	  //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	  SetColor4d(1.0, 1.0, 1.0, 1.0);
// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
	  col_object.r = 1.0;	col_object.g = 1.0;		col_object.b = 1.0;	 col_object.a = 1.0;
	  SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	  //glTranslated(-27.75f, 0.0f, 0.0f);
	  //glRotated(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScaled(10.0, 10.0, 5.0);
	  TransMatrix = glm::translate(MatriuTG, vec3(-27.75f, 0.0f, 0.0f));
	  TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	  ModelMatrix = glm::scale(TransMatrix, vec3(10.0f, 10.0f, 5.0f));
	  // Pas ModelView Matrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	  NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	  // Pas NormalMatrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	  draw_TriEBO_Object(GLU_CYLINDER); //gluCylinder(5.0f, 5.0f, 0.5f, 6, 1);
	//glPopMatrix();

//Tapas
	//glPushMatrix();
	  //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	  //glTranslatef(-27.75f, 0.0f, 0.0f);
	  //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScalef(10.0f, 10.0f, 5.0f);
	  TransMatrix = glm::translate(MatriuTG, vec3(-27.75f, 0.0f, 0.0f));
	  TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	  ModelMatrix = glm::scale(TransMatrix, vec3(10.0f, 10.0f, 5.0f));
	  // Pas ModelView Matrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	  NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	  // Pas NormalMatrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	  //gluDisk(0.0f, 5.0f, 6, 1);
	  draw_TriEBO_Object(GLU_DISK); //drawgluDisk(nvertD);
	//glPopMatrix();

	//glPushMatrix();
	  //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	  //glTranslatef(-25.25f, 0.0f, 0.0f);
	  //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScalef(10.0f, 10.0f, 5.0f);
	  TransMatrix = glm::translate(MatriuTG, vec3(-25.25f, 0.0f, 0.0f));
	  TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	  ModelMatrix = glm::scale(TransMatrix, vec3(10.0f, 10.0f, 5.0f));
	  // Pas ModelView Matrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	  NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	  // Pas NormalMatrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	  //gluDisk(0.0f, 5.0f, 6, 1);
	  draw_TriEBO_Object(GLU_DISK); //drawgluDisk(nvertD);
	//glPopMatrix();

//Parte Interior
	//glPushMatrix();
	  //glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	  // Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
	  col_object.r = 0.0;	col_object.g = 0.0;		col_object.b = 0.0;	 col_object.a = 1.0;
	  SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	  //glTranslatef(-29.25f, 0.0f, 0.0f);
	  //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScalef(2.0f, 2.0f, 2.0f);
	  TransMatrix = glm::translate(MatriuTG, vec3(-29.25f, 0.0f, 0.0f));
	  TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	  ModelMatrix = glm::scale(TransMatrix, vec3(2.0f, 2.0f, 2.0f));
	  // Pas ModelView Matrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	  NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	  // Pas NormalMatrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	  draw_TriEBO_Object(GLUT_USER1); // gluCylinder(5.0f, 5.0f, 2.0f, 6, 1);
	//glPopMatrix();

//Tapas
	//glPushMatrix();
	  //glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
 	  //glTranslatef(-29.25f, 0.0f, 0.0f);
	  //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScalef(2.0f, 2.0f, 2.0f);
	  TransMatrix = glm::translate(MatriuTG, vec3(-29.25f, 0.0f, 0.0f));
	  TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	  ModelMatrix = glm::scale(TransMatrix, vec3(2.0f, 2.0f, 2.0f));
	  // Pas ModelView Matrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	  NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	  // Pas NormalMatrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	  //gluDisk(0.0f, 5.0f, 6, 1);
	  draw_TriEBO_Object(GLU_DISK);  //drawgluDisk(nvertD);
	//glPopMatrix();

	//glPushMatrix();
	  //glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	  //SetColor4d(0.0, 0.0, 0.0, 0.0);
	  //glTranslatef(-25.25f, 0.0f, 0.0f);
	  //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScalef(2.0f, 2.0f, 2.0f);
	  TransMatrix = glm::translate(MatriuTG, vec3(-25.25f, 0.0f, 0.0f));
	  TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	  ModelMatrix = glm::scale(TransMatrix, vec3(2.0f, 2.0f, 2.0f));
	  // Pas ModelView Matrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	  NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	  // Pas NormalMatrix a shader
	  glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	  //gluDisk(0.0f, 5.0f, 6, 1);
	  draw_TriEBO_Object(GLU_DISK); //drawgluDisk(nvertD);
	//glPopMatrix();

//Decoracion Exterior
	int w;
	//glPushMatrix();
	  //glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	  //SetColor4d(0.0, 0.0, 0.0, 0.0);
	  //glTranslatef(-26.5f, 0.0f, 0.0f);
	  //glScalef(15.75f, 13.75f, 13.75f);
	  //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	  //glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
	  TransMatrix = glm::translate(MatriuTG, vec3(-26.5f, 0.0f, 0.0f));
	  TransMatrix = glm::scale(TransMatrix, vec3(15.75f, 13.75f, 13.75f));
	  TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	  TransMatrix = glm::rotate(TransMatrix, radians(30.f), vec3(0.0f, 0.0f, 1.0f));
	  for (w = 0; w<3; w = w + 1)
	  {	//glRotatef(60.0f, 0.0f, 0.0f, 1.0f);
		TransMatrix = glm::rotate(TransMatrix, radians(60.f), vec3(0.0f, 0.0f, 1.0f));
		//glPushMatrix();
			//glScalef(12.7f * 0.583f, 0.1f, 0.3f);
			ModelMatrix = glm::scale(TransMatrix, vec3(12.7f * 0.583f, 0.1f, 0.3f));
			// Pas ModelView Matrix a shader
			glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
			// Pas NormalMatrix a shader
			glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
			draw_TriEBO_Object(GLUT_CUBE); //glutSolidCube(1.0f);
		//glPopMatrix();
	}
	//glPopMatrix();

	int h;
	//glPushMatrix();
	  //glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	  //SetColor4d(0.0, 0.0, 0.0, 0.0);
	  TransMatrix = MatriuTG;
	  for (h = 0; h < 6; h = h + 1)
	  {	//glRotated(60, 1.0, 0.0, 0.0);
		TransMatrix = glm::rotate(TransMatrix, radians(60.f), vec3(1.0f, 0.0f, 0.0f));
		//glPushMatrix();
			//glTranslatef(-26.5f, 0.0f, 43.25f);
			//glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
			//glScalef(10 * 5.155f, 5.0f, 3.0f);
			ModelMatrix = glm::translate(TransMatrix, vec3(-26.5f, 0.0f, 43.25f));
			ModelMatrix = glm::rotate(ModelMatrix, radians(90.f), vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::scale(ModelMatrix, vec3(10 * 5.155f, 5.0f, 3.0f));
			// Pas ModelView Matrix a shader
			glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
			// Pas NormalMatrix a shader
			glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
			draw_TriEBO_Object(GLUT_CUBE); //glutSolidCube(1.0f);
		//glPopMatrix();
	  }
	//glPopMatrix();
	  //deleteVAO(GLU_DISK);
};


void Motor(GLint shaderId, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[4])
{
// Matrius  
	glm::mat4 TransMatrix(1.0), ModelMatrix(1.0), NormalMatrix(1.0);
	CColor col_object = { 0.0,0.0,0.0,1.0 };

//Helices Motor
	int j;
	//glPushMatrix();
	  //glColor4f(0.58f, 0.58f, 0.58f, 0.0f);
// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
	col_object.r = 0.58;	col_object.g = 0.58;	col_object.b = 0.58;	 col_object.a = 1.0;
	SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	//glTranslatef(0.0f, -18.0f, 0.0f);
	//glRotated(-90.0f, 0.0f, 0.0f, 1.0f);
	//glRotated(90.0f, 0.0f, 1.0f, 0.0f);
	//glRotatef(45, 0, 0, 1);
	TransMatrix = glm::translate(MatriuTG, vec3(0.0f, -18.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(-90.f), vec3(0.0f, 0.0f, 1.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(45.f), vec3(0.0f, 0.0f, 1.0f));
	for (j = 0; j<6; j = j + 1)
	{	//glRotatef(30, 0, 0, 1);
	TransMatrix = glm::rotate(TransMatrix, radians(30.f), vec3(0.0f, 0.0f, 1.0f));
	//glPushMatrix();
	//glScalef(12 * 0.583f, 0.1f, 0.3f);
	ModelMatrix = glm::scale(TransMatrix, vec3(12 * 0.583f, 0.1f, 0.3f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	// Pas NormalMatrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	draw_TriEBO_Object(GLUT_CUBE); //glutSolidCube(1.0f);
	//glPopMatrix();
	}
	//glPopMatrix();

//Soporte Motor
	//glPushMatrix();
	//glColor4f(0.28f, 0.28f, 0.28f, 0.0f);
	// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
	col_object.r = 0.28;	col_object.g = 0.28;	col_object.b = 0.28;	 col_object.a = 1.0;
	SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	//glTranslatef(0.0f, -18.0f, 0.0f);
	//glRotated(-90.0f, 0.0f, 0.0f, 1.0f);
	//glRotated(90.0f, 0.0f, 1.0f, 0.0f);
	TransMatrix = glm::translate(MatriuTG, vec3(0.0f, -18.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(-90.f), vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	// Pas NormalMatrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	draw_TriEBO_Object(GLUT_TORUS); //glutSolidTorus(1.0, 5.0, 20, 20);
	//glPopMatrix();
};

void Canon(GLint shaderId, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[4])
{
// Matrius Transformaci�
	glm::mat4 TransMatrix(1.0), ModelMatrix(1.0), NormalMatrix(1.0);
	CColor col_object = { 0.0,0.0,0.0,1.0 };

//Ca�ones

// Salida ca�on 1
	//glPushMatrix();
	//glColor4d(0.28, 0.28, 0.28, 0.0);
// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
	col_object.r = 0.28;	col_object.g = 0.28;	col_object.b = 0.28;	 col_object.a = 1.0;
	SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	//glTranslated(5.0, 8.0, -10.0);
	//glRotated(-90.0, 1.0, 0.0, 0.0);
	TransMatrix = glm::translate(MatriuTG, vec3(5.0f, 8.0f, -10.0f));
	ModelMatrix = glm::rotate(TransMatrix, radians(-90.f), vec3(1.0f, 0.0f, 0.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	// Pas NormalMatrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	draw_TriEBO_Object(GLUT_USER2); //gluCylinder(1.0f, 0.5f, 5.0f, 60, 1);
	//glPopMatrix();

// Salida ca�on 2
	//glPushMatrix();
	//glColor4d(0.28, 0.28, 0.28, 0.0);
	//glTranslated(-5.0, 8.0, -10.0);
	//glRotated(-90.0, 1.0, 0.0, 0.0);
	TransMatrix = glm::translate(MatriuTG, vec3(-5.0f, 8.0f, -10.0f));
	ModelMatrix = glm::rotate(TransMatrix, radians(-90.f), vec3(1.0f, 0.0f, 0.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	// Pas NormalMatrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	draw_TriEBO_Object(GLUT_USER2); // gluCylinder(1.0f, 0.5f, 5.0f, 60, 1);
	//glPopMatrix();

// Ca�on 1
	//glPushMatrix();
		//glColor4d(0.58, 0.58, 0.58, 0.0);
// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
	col_object.r = 0.58;	col_object.g = 0.58;		col_object.b = 0.58;	 col_object.a = 1.0;
	SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	//glTranslated(5.0, 10.0, -10.0);
	//glRotated(-90.0, 1.0, 0.0, 0.0);
	TransMatrix = glm::translate(MatriuTG, vec3(5.0f, 10.0f, -10.0f));
	ModelMatrix = glm::rotate(TransMatrix, radians(-90.f), vec3(1.0f, 0.0f, 0.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	// Pas NormalMatrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	draw_TriEBO_Object(GLUT_USER3); // gluCylinder(0.35f, 0.35f, 5.0f, 80, 1);
	//glPopMatrix();

//Ca�on 2
	//glPushMatrix();
	//glColor4d(0.58, 0.58, 0.58, 1.0);
 	//glTranslated(-5.0, 10.0, -10.0);
	//glRotated(-90.0, 1.0, 0.0, 0.0);
	TransMatrix = glm::translate(MatriuTG, vec3(-5.0f, 10.0f, -10.0f));
	ModelMatrix = glm::rotate(TransMatrix, radians(-90.f), vec3(1.0f, 0.0f, 0.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	// Pas NormalMatrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	draw_TriEBO_Object(GLUT_USER3); // gluCylinder(0.35f, 0.35f, 5.0f, 80, 1);
	//glPopMatrix();
}

void Cuerpo(GLint shaderId, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[4])
{
// Matrius Transformaci�
	glm::mat4 TransMatrix(1.0), ModelMatrix(1.0), NormalMatrix(1.0);
	CColor col_object = { 0.0,0.0,0.0,1.0 };

// Activar transpar�ncia
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//Sujeccion de las Alas

//Lado2
	//glPushMatrix();
	//glColor4d(0.16, 0.16, 0.16, 1.0);
// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
	col_object.r = 0.16;	col_object.g = 0.16;	col_object.b = 0.16;	 col_object.a = 0.5;
	SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	//glRotated(-90.0, 0.0, 1.0, 0.0);
	//glRotated(90.0, 0.0, 0.0, 1.0);
	//glScaled(2.0, 2.0, 2.5);
	TransMatrix = glm::rotate(MatriuTG, radians(-90.f), vec3(0.0f, 1.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(2.0f, 2.0f, 2.5f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	// Pas NormalMatrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	draw_TriEBO_Object(GLUT_USER4); // gluCylinder(4.0f, 2.0f, 10.25f, 40, 1);
	//glPopMatrix();

//X2
	//glPushMatrix();
	//glColor4d(0.16, 0.16, 0.16, 1.0);
	//glTranslated(-25.0, 0.0, 0.0);
	//glRotated(90.0, 0.0, 1.0, 0.0);
	TransMatrix = glm::translate(MatriuTG, vec3(-25.0f, 0.0f, 0.0f));
	ModelMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	// Pas NormalMatrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	draw_TriEBO_Object(GLUT_TORUS); // glutSolidTorus(1.0, 5.0, 20, 20);
	//glPopMatrix();

//Lado 1
	//glPushMatrix();
	//glColor4f(0.16f, 0.16f, 0.16f, 1.0f);
	//SetColor4d(0.16, 0.16, 0.16, 1.0);
	//glRotated(90.0, 0.0, 1.0, 0.0);
	//glRotated(90.0, 0.0, 0.0, 1.0);
	//glScaled(2.0, 2.0, 2.5);
	TransMatrix = glm::rotate(MatriuTG, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(2.0f, 2.0f, 2.5f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	// Pas NormalMatrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	draw_TriEBO_Object(GLUT_USER4); // gluCylinder(4.0f, 2.0f, 10.25f, 40, 1);
	//glPopMatrix();

//X1
	//glPushMatrix();
	//glColor4d(0.16, 0.16, 0.16, 1.0);
	//SetColor4d(0.16, 0.16, 0.16, 1.0);
	//glTranslated(25.25, 0.0, 0.0);
	//glRotated(90.0, 0.0, 1.0, 0.0);
	TransMatrix = glm::translate(MatriuTG, vec3(25.25f, 0.0f, 0.0f));
	ModelMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	// Pas NormalMatrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	draw_TriEBO_Object(GLUT_TORUS); //glutSolidTorus(1.0, 5.0, 20, 20);
	//glPopMatrix();

//Cuerpo
	//glPushMatrix();
	//glColor4d(0.16, 0.16, 0.16, 1.0);
	//SetColor4d(0.16, 0.16, 0.16, 1.0);
	//glScaled(1.75f, 1.75f, 1.5f);
	ModelMatrix = glm::scale(MatriuTG, vec3(1.75f, 1.75f, 1.5f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	// Pas NormalMatrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	draw_TriEBO_Object(GLU_SPHERE); //gluSphere(10.0f, 80, 80);
	//glPopMatrix();

 // Desactivar transpar�ncia
	glDisable(GL_BLEND);
}

void Cabina(GLint shaderId, glm::mat4 MatriuVista, glm::mat4 MatriuTG, bool sw_mat[4])
{
// Matrius Transformaci�
	glm::mat4 TransMatrix(1.0), ModelMatrix(1.0), NormalMatrix(1.0);
	CColor col_object = { 0.0,0.0,0.0,1.0 };

// Activar transpar�ncia
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

// Tapa Cabina
	//glPushMatrix();
	//glColor4d(1.0, 0.058, 0.058, 0.5);
	// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
	col_object.r = 1.0;	col_object.g = 0.058;	col_object.b = 0.058;	 col_object.a = 0.5;
	SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	//glTranslated(0.0, 19.45, 0.0);
	//glRotated(-90.0, 0.0, 0.0, 1.0);
	//glRotated(90.0, 0.0, 1.0, 0.0);
	//glScaled(1.75, 1.75, 1.75);
	TransMatrix = glm::translate(MatriuTG, vec3(0.0f, 19.45f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(-90.f), vec3(0.0f, 0.0f, 1.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(1.75f, 1.75f, 1.75f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	// Pas NormalMatrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	draw_TriEBO_Object(GLUT_USER6); //gluDisk(0.0f, 1.5f, 8, 1);
	//glPopMatrix();

// Cristal Cabina
	//glPushMatrix();
	//glColor4d(1.0, 0.058, 0.058, 0.5);
	//glTranslated(0.0f, 19.45f, 0.0f);
	//glRotated(-90.0f, 0.0f, 0.0f, 1.0f);
	//glRotated(90.0f, 0.0f, 1.0f, 0.0f);
	//glScaled(1.75f, 1.75f, 1.75f);
	TransMatrix = glm::translate(MatriuTG, vec3(0.0f, 19.45f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(-90.f), vec3(0.0f, 0.0f, 1.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(1.75f, 1.75f, 1.75f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
	// Pas NormalMatrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
	draw_TriEBO_Object(GLUT_USER5); //gluCylinder(1.5f, 4.5f, 2.0f, 8, 1);
	//glPopMatrix();

// Desactivar transpar�ncia
	glDisable(GL_BLEND);
}
// FI OBJECTE TIE: FETS PER ALUMNES -----------------------------------------------------------------
