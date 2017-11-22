// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <stack>   
#include <sstream>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "common/tga.h"
#include "common/tga.c"
#include "common/ray_casting.h"
using namespace glm;
// Include AntTweakBar
#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

const int window_width = 600, window_height = 600;


typedef struct Vertex {
	float Position[4];
	float Color[4];
	float Normal[3];
	float Tex[2];
	void SetPosition(float *coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];
		Position[3] = 1.0;
	}
	void SetColor(float *color) {
		Color[0] = color[0];
		Color[1] = color[1];
		Color[2] = color[2];
		Color[3] = color[3];
	}
	void SetNormal(float *coords) {
		Normal[0] = coords[0];
		Normal[1] = coords[1];
		Normal[2] = coords[2];
	}
	void SetTex(float *texco) { //texture coordinates 
		Tex[0] = texco[0];
		Tex[1] = texco[1];
	}
};

// function prototypes
int initWindow(void);
void initOpenGL(void);
void loadObject(char*, glm::vec4, Vertex * &, GLuint* &, int);
void createVAOs(Vertex[], GLuint[], int);
void createObjects(void);
void pickObject(void);
void renderScene(void);
void cleanup(void);
static void keyCallback(GLFWwindow*, int, int, int, int);
static void mouseCallback(GLFWwindow*, int, int, int);
void raycast(Vertex[], GLuint[]);


// GLOBAL VARIABLES
GLFWwindow* window;
float pickingColor[441];

GLint ambval1;
GLint ambval2;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex = -1;
std::string gMessage;

GLuint programID;
GLuint pickingProgramID;

const GLuint NumObjects = 8;	// ATTN: THIS NEEDS TO CHANGE AS YOU ADD NEW OBJECTS
GLuint VertexArrayId[NumObjects] = { 0,1,2,3,4,5,6,7 };
GLuint VertexBufferId[NumObjects] = { 0,1,2,3,4,5,6,7 };
GLuint IndexBufferId[NumObjects] = { 0,1,2,3,4,5,6,7 };
GLuint* CIndices;
Vertex *Control;
GLuint *GXI, *GYI;

size_t NumIndices[NumObjects] = { 0,0,0,0,0,0,0 };
size_t VertexBufferSize[NumObjects] = { 0,0,0,0,0,0,0 };
size_t IndexBufferSize[NumObjects] = { 0,0,0,0,0,0,0 };

GLuint MatrixID;
GLuint ModelMatrixID;
GLuint ViewMatrixID;
GLuint ProjMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorArrayID;
GLuint pickingColorID;
GLuint textureProgramID;

GLuint texModelMatID;
GLuint texViewMatID;
GLuint texProjMatID;

GLuint textureMatrixID;

GLuint texLoc;
GLuint teximage;

GLfloat Tex[882];

GLuint LightID;
GLuint LightID2;

GLint gX = 0.0;
GLint gZ = 0.0;

int outvertcount;
int outidcount;
Vertex* Verts;
GLuint* Idcs;

bool shift = false;
bool sKey = false;
bool lKey = false;

// animation control
bool animation = false;
GLfloat phi = 0.0;


unsigned int pickedID;

//camera
float M_PI = 3.14159265358979323846;  /* pi */
GLfloat cameraAngleTheta = M_PI / 4;
GLfloat cameraAnglePhi = asin(1 / (sqrt(3)));
GLfloat cameraSphereRadius = sqrt(300);
bool cmesh = false;
bool face = false;
bool reset = false;
bool selectCamera = true;
bool moveCameraLeft = false;
bool moveCameraRight = false;
bool moveCameraUp = false;
bool moveCameraDown = false;
bool raycasting = false;
bool smileKey = false;
Vertex temp[10];
void loadObject(char* file, glm::vec4 color, Vertex * &out_Vertices, GLuint* &out_Indices, int ObjectId)
{
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ(file, vertices, normals);

	std::vector<GLushort> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, normals, indices, indexed_vertices, indexed_normals);

	const size_t vertCount = indexed_vertices.size();
	outvertcount = vertCount;
	const size_t idxCount = indices.size();
	outidcount = idxCount;

	// populate output arrays
	out_Vertices = new Vertex[vertCount];
	for (int i = 0; i < vertCount; i++) {
		out_Vertices[i].SetPosition(&indexed_vertices[i].x);
		out_Vertices[i].SetNormal(&indexed_normals[i].x);
		out_Vertices[i].SetColor(&color[0]);
	}
	out_Indices = new GLuint[idxCount];
	for (int i = 0; i < idxCount; i++) {
		out_Indices[i] = indices[i];
	}

	// set global variables!!
	NumIndices[ObjectId] = idxCount;
	VertexBufferSize[ObjectId] = sizeof(out_Vertices[0]) * vertCount;
	IndexBufferSize[ObjectId] = sizeof(GLuint) * idxCount;
}

void create_mesh()
{



	//control mesh

	int Cpts = 441;
	Control = new Vertex[Cpts];

	int i;
	float clr[] = { 0.0f,5.0f,0.0f,1.0f };
	float lclr[] = { 2.0f,2.0f,2.0f,2.0f };
	float n[] = { 0.0f,0.0f,1.0f };
	float pos[3];
	float texpos[2];
	float x = -5.5f;
	float y = -0.5f;
	int j, k;


	for (i = 0; i < Cpts;)
	{
		k = 0.0f;
		while (k < 21.0f)
		{
			j = 0.0f;
			while (j < 21.0f)
			{
				pos[0] = x + 0.5f * j;
				pos[1] = y + 0.5f * k;
				pos[2] = -0.5f; //Z position
				texpos[1] = j / 20.0f;
				texpos[0] = k / 20.0f;
				Control[i].SetColor(clr);
				Control[i].SetNormal(n);
				Control[i].SetPosition(pos);
				Control[i].SetTex(texpos);
				i++;
				j++;
			}
			k++;
		}
	}

	CIndices = new GLuint[2400];

	j = 0;
	i = 0;

	while (i<2400)
	{
		k = 0;
		while (k < 20)
		{
			CIndices[i] = k + j;
			CIndices[i + 1] = k + 1 + j;
			CIndices[i + 2] = k + 22 + j;
			CIndices[i + 3] = k + 22 + j;
			CIndices[i + 4] = k + 21 + j;
			CIndices[i + 5] = k + j;
			i += 6;
			k += 1;
		}
		j += 21;
	}

	GXI = new GLuint[840];
	i = 0;
	k = 0;
	while (k < 441)
	{
		j = 0;
		while (j < 20)
		{
			GXI[i] = j + k;
			GXI[i + 1] = j + 1 + k;
			i += 2;
			j++;
		}
		k += 21;
	}

	GYI = new GLuint[840];
	i = 0;
	k = 0;
	while (k < 420)
	{
		j = 0;
		while (j < 21)
		{
			GYI[i] = j + k;
			GYI[i + 1] = j + 21 + k;
			i += 2;
			j++;
		}
		k += 21;
	}


	NumIndices[3] = 2400;
	VertexBufferSize[3] = sizeof(Vertex) * Cpts;
	IndexBufferSize[3] = sizeof(GLuint) * 2400;

	//raycast(Verts, Idcs);

	createVAOs(Control, CIndices, 3);

	for (i = 0; i < Cpts; i++)
	{
		Control[i].SetColor(lclr);
	}

	NumIndices[4] = 840;
	VertexBufferSize[4] = sizeof(Vertex) * Cpts;
	IndexBufferSize[4] = sizeof(GLuint) * 840;

	createVAOs(Control, GXI, 4);

	NumIndices[5] = 840;
	VertexBufferSize[5] = sizeof(Vertex) * Cpts;
	IndexBufferSize[5] = sizeof(GLuint) * 840;

	createVAOs(Control, GYI, 5);
	//texture//
	NumIndices[6] = 2400;
	VertexBufferSize[6] = sizeof(Vertex) * Cpts;
	IndexBufferSize[6] = sizeof(GLuint) * 2400;
	createVAOs(Control, CIndices, 6);

	for (i = 0; i < Cpts; i++)
	{
		Control[i].SetColor(clr);
	}
}

void createObjects(void)
{
	//-- COORDINATE AXES --//
	Vertex CoordVerts[] =
	{
		{ { 0.0, 0.0, 0.0, 1.0 },{ 1.0, 0.0, 0.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 0.0, 1.0 },{ 1.0, 0.0, 0.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 },{ 0.0, 1.0, 0.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 0.0, 5.0, 0.0, 1.0 },{ 0.0, 1.0, 0.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 },{ 0.0, 0.0, 1.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 5.0, 1.0 },{ 0.0, 0.0, 1.0, 1.0 },{ 0.0, 0.0, 1.0 } },
	};

	VertexBufferSize[0] = sizeof(CoordVerts);	// ATTN: this needs to be done for each hand-made object with the ObjectID (subscript)
	createVAOs(CoordVerts, NULL, 0);

	//-- GRID --//
	Vertex GRID[] =
	{
		{ { -5.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -4.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -4.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -4.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -4.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -3.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -3.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -3.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -3.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -2.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -2.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -2.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -2.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -1.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -1.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -1.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -1.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -0.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -0.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 0.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 0.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 1.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 1.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 1.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 1.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 2.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 2.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 2.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 2.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 3.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 3.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 3.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 3.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 4.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 4.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 4.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 4.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },

		{ { -5.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -4.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -4.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -4.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -4.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -3.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -3.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -3.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -3.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -2.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -2.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -2.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -2.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -1.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -1.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -1.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -1.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -0.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -0.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 0.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 0.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 0.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 0.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 1.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 1.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 1.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 1.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 2.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 2.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 2.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 2.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 3.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 3.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 3.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 3.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 4.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 4.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 4.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 4.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
	};

	VertexBufferSize[1] = sizeof(GRID);	// ATTN: this needs to be done for each hand-made object with the ObjectID (subscript)
	createVAOs(GRID, NULL, 1);
	// ATTN: create your grid vertices here!

	loadObject("suman.obj", glm::vec4(0.55, 0.2, 0.2, 1.0), Verts, Idcs, 2);
	createVAOs(Verts, Idcs, 2);

	create_mesh();

	//-- .OBJs --//

	// ATTN: load your models here
	loadObject("new1.obj", glm::vec4(0.0, 0.0, 0.0, 1.0), Verts, Idcs, 7);
	createVAOs(Verts, Idcs, 7);

}



void raycast(Vertex modelverts[], unsigned int modelIndices[]) {
	float  v1[3], v2[3], v3[3], cp[3], bary[3];
	float dir[] = { 0,0,1 };
	float col;
	for (int a = 0; a<441; a++) {
		cp[0] = Control[a].Position[0];
		cp[1] = Control[a].Position[1];
		cp[2] = Control[a].Position[2];

		float baryz;
		for (int i = 0; i<(outidcount - 2); i++) {
			v1[0] = modelverts[modelIndices[i]].Position[0];
			v1[1] = modelverts[modelIndices[i]].Position[1];
			v1[2] = modelverts[modelIndices[i]].Position[2];

			v2[0] = modelverts[modelIndices[i + 1]].Position[0];
			v2[1] = modelverts[modelIndices[i + 1]].Position[1];
			v2[2] = modelverts[modelIndices[i + 1]].Position[2];

			v3[0] = modelverts[modelIndices[i + 2]].Position[0];
			v3[1] = modelverts[modelIndices[i + 2]].Position[1];
			v3[2] = modelverts[modelIndices[i + 2]].Position[2];

			ray_cast(v1, v2, v3, cp, dir, bary);


			if ((bary[0] >= 0.0) && (bary[1] >= 0.0) && (bary[2] >= 0.0)) {
				baryz = bary[0] * v1[2] + bary[1] * v2[2] + bary[2] * v3[2];
				if (baryz != INFINITY)
					col = baryz;
				if ((Control[a].Position[2] < col)) {
					Control[a].Position[2] = col;
					//printf("%f,%f,%f\n", Control[a].Position[0], Control[a].Position[1], Control[a].Position[2]);
				}
			}
		}
	}
	printf("DONE\n");
}

void renderScene(void)
{
	//ATTN: DRAW YOUR SCENE HERE. MODIFY/ADAPT WHERE NECESSARY!

	// Dark Blue background
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//SNipet
	if (moveCameraLeft) {
		cameraAngleTheta -= 0.01f;
	}

	if (moveCameraRight) {
		cameraAngleTheta += 0.01f;
	}

	if (moveCameraUp) {
		cameraAnglePhi += 0.01f;
	}

	if (moveCameraDown) {
		cameraAnglePhi -= 0.01f;
	}

	if (selectCamera && (moveCameraLeft || moveCameraRight || moveCameraDown || moveCameraUp)) {
		float camX = cameraSphereRadius * cos(cameraAnglePhi) * sin(cameraAngleTheta);
		float camY = cameraSphereRadius * sin(cameraAnglePhi);
		float camZ = cameraSphereRadius * cos(cameraAnglePhi) * cos(cameraAngleTheta);
		gViewMatrix = glm::lookAt(glm::vec3(camX, camY, camZ),	// eye
			glm::vec3(0.0, 0.0, 0.0),	// center
			glm::vec3(0.0, 1.0, 0.0));	// up


	}
	if (reset)
	{
		cameraAngleTheta = M_PI / 4;
		cameraAnglePhi = asin(1 / (sqrt(3)));
		cameraSphereRadius = sqrt(300);
		float camX = cameraSphereRadius * cos(cameraAnglePhi) * sin(cameraAngleTheta);
		float camY = cameraSphereRadius * sin(cameraAnglePhi);
		float camZ = cameraSphereRadius * cos(cameraAnglePhi) * cos(cameraAngleTheta);
		gViewMatrix = glm::lookAt(glm::vec3(camX, camY, camZ),	// eye
			glm::vec3(0.0, 0.0, 0.0),	// center
			glm::vec3(0.0, 1.0, 0.0));	// up
		create_mesh();
	}
	glUseProgram(programID);
	{
		glm::vec3 lightPos = glm::vec3(-15, 10, 9);
		glm::vec3 lightPos2 = glm::vec3(-5, 10, 10);
		glm::mat4x4 ModelMatrix = glm::mat4(1.0);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(LightID2, lightPos2.x, lightPos2.y, lightPos2.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glUniform3f(ambval1, 0.5f, 0.5f, 0.5f);
		glUniform3f(ambval2, 0.5f, 0.5f, 0.5f);

		glBindVertexArray(VertexArrayId[0]);	// draw CoordAxes
		glDrawArrays(GL_LINES, 0, 6);
		glBindVertexArray(0);

		glBindVertexArray(VertexArrayId[1]);	//draw grid
		glDrawArrays(GL_LINES, 0, 88);
		glBindVertexArray(1);

		if (face)
		{
			glUniform3f(ambval1, 0.5, 0.5, 0.5);
			glUniform3f(ambval2, 0.5, 0.5, 0.5);

			glBindVertexArray(VertexArrayId[2]);	//draw face
			glDrawElements(GL_TRIANGLES, NumIndices[2], GL_UNSIGNED_INT, (void*)0);
			glBindVertexArray(2);

			glBindVertexArray(VertexArrayId[7]);	//draw hair
			glDrawElements(GL_TRIANGLES, NumIndices[7], GL_UNSIGNED_INT, (void*)0);
			glBindVertexArray(7);
		}
		if (smileKey)
		{
			temp[0] = Control[94];
			temp[1] = Control[96];
			Control[94].Position[0] = Control[94].Position[0] - 0.01f;
			Control[94].Position[1] = Control[94].Position[1] + 0.01f;
			Control[96].Position[0] = Control[96].Position[0] + 0.01f;
			Control[95].Position[1] = Control[95].Position[1] + 0.00001f;
			Control[85].Position[1] = Control[85].Position[1] + 0.00001f;
			Control[96].Position[1] = Control[96].Position[1] + 0.01f;
			Control[94].Position[2] = Control[94].Position[2] - 0.0001f;
			Control[96].Position[2] = Control[96].Position[2] - 0.0001f;
			glm::mat4 ModelMatrix = glm::mat4(1.0);
			glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		}
		if (cmesh)
		{

			glPointSize(3.0);

			glBindVertexArray(VertexArrayId[4]);	//draw control mesh
			glDrawElements(GL_LINES, NumIndices[4], GL_UNSIGNED_INT, (void*)0);
			glBindVertexArray(4);

			glBindVertexArray(VertexArrayId[5]);	//draw control mesh
			glDrawElements(GL_LINES, NumIndices[5], GL_UNSIGNED_INT, (void*)0);
			glBindVertexArray(5);

			// Save To File
			if (sKey)
			{
				FILE *fp;
				fp = fopen("cm.p3", "w+");
				printf("Writing in Progress!");
				for (int i = 0; i < 441; i++)
				{
					for (int j = 0; j < 4; j++) {
						fprintf(fp, "%0.2f", Control[i].Position[j]);
						fprintf(fp, "%s", ",");
					}
					for (int j = 0; j < 4; j++) {
						fprintf(fp, "%0.2f", Control[i].Color[j]);
						fprintf(fp, "%s", ",");
					}
					for (int j = 0; j < 3; j++) {
						fprintf(fp, "%0.2f", Control[i].Normal[j]);
						fprintf(fp, "%s", ",");
					}
					if (i != (440))fprintf(fp, "%s", "\n");
				}
				fprintf(fp, "%s", "p");
				fclose(fp);
				sKey = false;
				printf("\nWriting Completed\n");
			}

			//Load From File
			if (lKey == 1)
			{
				//char fname[10];
				//printf("Enter the name of the file");
				//cin >> fname;
				FILE *fp;
				fp = fopen("cm.p3", "r");
				if (!fp)
					printf("no such file");
				char cRead = NULL;
				char*  vertexValue = (char*)malloc(6 * sizeof(char));
				int pos = 0, cnt = 0, j = 0;
				float floatValofVertex = 0;
				int i = 0;
				while (cRead != 'p') {
					cRead = (char)fgetc(fp);
					if (cRead == ',')
					{
						j = 0;
						floatValofVertex = (float)atof(vertexValue);
						if (cnt == 0)
						{
							Control[pos].Position[0] = floatValofVertex;
						}
						else if (cnt == 1)
						{
							Control[pos].Position[1] = floatValofVertex;
						}
						else if (cnt == 2)
						{
							Control[pos].Position[2] = floatValofVertex;
						}
						else if (cnt == 3)
						{
							Control[pos].Position[3] = floatValofVertex;
						}
						else if (cnt == 4)
						{
							Control[pos].Color[0] = floatValofVertex;
						}
						else if (cnt == 5)
						{
							Control[pos].Color[1] = floatValofVertex;
						}
						else if (cnt == 6)
						{
							Control[pos].Color[2] = floatValofVertex;
						}
						else if (cnt == 7)
						{
							Control[pos].Color[3] = floatValofVertex;
						}
						else if (cnt == 8)
						{
							Control[pos].Normal[0] = floatValofVertex;
						}
						else if (cnt == 9)
						{
							Control[pos].Normal[1] = floatValofVertex;
						}
						else if (cnt == 10)
						{
							Control[pos].Normal[2] = floatValofVertex;
						}
						cnt++;
						if (cnt == 11)
							pos++;
						continue;
					}
					else if (cRead != '\n' && cRead != 'p') {
						vertexValue[j] = cRead;
						j++;
					}
					if (cRead == '\n') {
						cnt = 0;
					}
				}
				fclose(fp);
				lKey = false;
				printf("\nReading Completed: Control[0] value : %0.2f\n", Control[0].Position[0]);
				createVAOs(Control, CIndices, 3);
			}
			glBindVertexArray(VertexArrayId[3]);	//draw control mesh
			glDrawElements(GL_POINTS, NumIndices[3], GL_UNSIGNED_INT, (void*)0);
			glBindVertexArray(3);

			//draw grid 
			int i;
			float clr[] = { 0.0f,5.0f,0.0f,1.0f };
			float lclr[] = { 2.0f,2.0f,2.0f,2.0f };

			for (i = 0; i < 441; i++)
			{
				Control[i].SetColor(lclr);
			}

			NumIndices[4] = 840;
			VertexBufferSize[4] = sizeof(Vertex) * 441;
			IndexBufferSize[4] = sizeof(GLuint) * 840;

			createVAOs(Control, GXI, 4);

			NumIndices[5] = 840;
			VertexBufferSize[5] = sizeof(Vertex) * 441;
			IndexBufferSize[5] = sizeof(GLuint) * 840;

			createVAOs(Control, GYI, 5);

			for (i = 0; i < 441; i++)
			{
				Control[i].SetColor(clr);
			}

		}


	}
	glUseProgram(0);
	if (cmesh) {
		glUseProgram(textureProgramID); {   //TEXTURE
			glm::mat4x4 ModelMatrix = glm::mat4(1.0);
			glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
			glUniformMatrix4fv(textureMatrixID, 1, GL_FALSE, &MVP[0][0]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, teximage);
			glUniform1i(texLoc, 0);
			glBindVertexArray(VertexArrayId[6]);
			glDrawElements(GL_TRIANGLES, NumIndices[6], GL_UNSIGNED_INT, (void*)0);
		}
	}
	glUseProgram(0);
	// Draw GUI
	TwDraw();

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

vec3 FCP(vec3 rayStartPos, vec3 rayEndPos, vec3 pointPos, double *proj)
{
	vec3 rayVector = rayEndPos - rayStartPos;
	double raySquared = glm::dot(rayVector, rayVector);
	vec3 projection = pointPos - rayStartPos;
	double projectionVal = glm::dot(projection, rayVector);
	*proj = projectionVal / raySquared;
	vec3 closestPoint = rayStartPos + glm::vec3(rayVector.x * (*proj), rayVector.y * (*proj), rayVector.z * (*proj));
	return closestPoint;
}

bool raytracing(vec3 pointPos, vec3 startPos, vec3 endPos, vec3 *closestPoint, double *proj, double epsilon)
{
	*closestPoint = FCP(startPos, endPos, pointPos, proj);
	double len = glm::distance2(*closestPoint, pointPos);
	return len < epsilon;
}

bool raytracingpts(Vertex* vert, vec3 start, vec3 end, unsigned int *id, double *proj, double epsilon, int maxRange)
{
	unsigned int pointID = maxRange + 1;
	bool foundCollision = false;
	double minDistToStart = 10000000.0;
	double distance;
	vec3 point;

	for (unsigned int i = 0; i<maxRange; ++i)
	{
		vec3 pointPos = vec3(vert[i].Position[0], vert[i].Position[1], vert[i].Position[2]);

		if (raytracing(pointPos, start, end, &point, proj, epsilon))
		{
			distance = glm::distance2(start, point);
			if (distance<minDistToStart)
			{
				minDistToStart = distance;
				pointID = i;
				foundCollision = true;
			}
		}
	}

	*id = pointID;
	return foundCollision;

}

void pickObject(void)
{
	// Clear the screen in lclr
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 ModelMatrix = glm::mat4(1.0);

	glUseProgram(pickingProgramID);
	{
		// TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);

		// ATTN: DRAW YOUR PICKING SCENE HERE. REMEMBER TO SEND IN A DIFFERENT PICKING COLOR FOR EACH OBJECT BEFOREHAND
		//	glPointSize(5.0);

		//	glUniform1fv(pickingColorArrayID, 441, pickingColor);
		//	glBindVertexArray(VertexArrayId[3]);	//draw points
		//	glDrawElements(GL_POINTS, NumIndices[3], GL_UNSIGNED_INT, (void*)0);
		//	glBindVertexArray(3);
		glBindVertexArray(0);

	}
	glUseProgram(0);
	// Wait until all the pending drawing commands are really done.
	// Ultra-mega-over slow ! 
	// There are usually a long time between glDrawElements() and
	// all the fragments completely rasterized.
	glFlush();
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Read the pixel at the center of the screen.
	// You can also use glfwGetMousePos().
	// Ultra-mega-over slow too, even for 1 pixel, 
	// because the framebuffer is on the GPU.

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	unsigned char data[4];
	glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	mat4 nModelMatrix = gViewMatrix * ModelMatrix;

	vec3 startMousePos = glm::unProject(vec3(xpos, window_height - ypos, 0.0f), nModelMatrix, gProjectionMatrix, vec4(viewport[0], viewport[1], viewport[2], viewport[3]));
	vec3 endMousePos = glm::unProject(vec3(xpos, window_height - ypos, 1.0f), nModelMatrix, gProjectionMatrix, vec4(viewport[0], viewport[1], viewport[2], viewport[3]));

	double epsilon = 0.1;
	double proj;
	bool found = raytracingpts(Control, startMousePos, endMousePos, &pickedID, &proj, epsilon, 441);

	if (pickedID>440)
	{
		pickedID = 510;
	}
	// Convert the color back to an integer ID
	//gPickedIndex = int(data[0]) + int(data[1]);
	gPickedIndex = pickedID;

	if (gPickedIndex == 510) { // Full lclr, must be the background !
		gMessage = "background";
	}
	else {
		std::ostringstream oss;
		oss << "point " << pickedID;
		gMessage = oss.str();
	}

	// Uncomment these lines to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the normal rendering
}

void moveVertex()
{
	GLint viewport[4];
	glm::mat4 ModelMatrix = glm::mat4(1.0);
	glGetIntegerv(GL_VIEWPORT, viewport);
	glm::vec4 vp = glm::vec4(viewport[0], viewport[1], viewport[2], viewport[3]);

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	mat4 nModelMatrix = gViewMatrix * ModelMatrix;

	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	vec3 W;
	vec3 p;
	float coords[3];

	if (state == GLFW_PRESS)
	{
		if (shift)
		{
			p = glm::project(vec3(Control[pickedID].Position[0], Control[pickedID].Position[1], Control[pickedID].Position[2]), nModelMatrix, gProjectionMatrix, vp);
			W = glm::unProject(vec3(xpos, window_height - ypos, p.z), nModelMatrix, gProjectionMatrix, vec4(viewport[0], viewport[1], viewport[2], viewport[3]));
			coords[0] = Control[pickedID].Position[0];
			coords[1] = Control[pickedID].Position[1];
			coords[2] = W.z;
			Control[pickedID].SetPosition(coords);
		}
		else
		{

			p = glm::project(vec3(Control[pickedID].Position[0], Control[pickedID].Position[1], Control[pickedID].Position[2]), nModelMatrix, gProjectionMatrix, vp);
			W = glm::unProject(vec3(xpos, window_height - ypos, p.z), nModelMatrix, gProjectionMatrix, vec4(viewport[0], viewport[1], viewport[2], viewport[3]));
			coords[0] = W.x;
			coords[1] = W.y;
			coords[2] = Control[pickedID].Position[2];
			Control[pickedID].SetPosition(coords);
		}

	}
	NumIndices[3] = 2400;
	VertexBufferSize[3] = sizeof(Vertex) * 441;
	IndexBufferSize[3] = sizeof(GLuint) * 2400;

	createVAOs(Control, CIndices, 3);

	int i;
	float clr[] = { 0.0f,5.0f,0.0f,1.0f };
	float lclr[] = { 2.0f,2.0f,2.0f,2.0f };

	for (i = 0; i < 441; i++)
	{
		Control[i].SetColor(lclr);
	}

	NumIndices[4] = 840;
	VertexBufferSize[4] = sizeof(Vertex) * 441;
	IndexBufferSize[4] = sizeof(GLuint) * 840;

	createVAOs(Control, GXI, 4);

	NumIndices[5] = 840;
	VertexBufferSize[5] = sizeof(Vertex) * 441;
	IndexBufferSize[5] = sizeof(GLuint) * 840;

	createVAOs(Control, GYI, 5);

	NumIndices[6] = 2400;
	VertexBufferSize[6] = sizeof(Vertex) * 441;
	IndexBufferSize[6] = sizeof(GLuint) * 2400;
	createVAOs(Control, CIndices, 6);

	for (i = 0; i < 441; i++)
	{
		Control[i].SetColor(clr);
	}
}

int initWindow(void)
{
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(window_width, window_height, "Singh_Suman (36395801)", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Initialize the GUI
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(window_width, window_height);
	TwBar * GUI = TwNewBar("Picking");
	TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
	TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);

	// Set up inputs
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);

	return 0;
}

void initOpenGL(void)
{

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	gProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	gViewMatrix = glm::lookAt(glm::vec3(10.0, 10.0, 10.0f),	// eye
		glm::vec3(0.0, 0.0, 0.0),	// center
		glm::vec3(0.0, 1.0, 0.0));	// up

									// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");
	textureProgramID = LoadShaders("texture.vertexshader", "texture.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ProjMatrixID = glGetUniformLocation(programID, "P");
	textureMatrixID = glGetUniformLocation(textureProgramID, "MVP");

	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
	pickingColorArrayID = glGetUniformLocation(pickingProgramID, "PickingColorArray");
	// Get a handle for our "LightPosition" uniform
	ambval1 = glGetUniformLocation(programID, "amb1");
	ambval2 = glGetUniformLocation(programID, "amb2");
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	LightID2 = glGetUniformLocation(programID, "LightPosition_worldspace1");

	texLoc = glGetUniformLocation(textureProgramID, "myTextureSampler");
	long wid, ht;
	teximage = load_texture_TGA("singh_suman.tga", &wid, &ht);

	createObjects();
}

void createVAOs(Vertex Vertices[], unsigned int Indices[], int ObjectId) {

	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);
	const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;
	const size_t TexOffset = sizeof(Vertices[0].Normal) + Normaloffset;

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);	//
	glBindVertexArray(VertexArrayId[ObjectId]);		//

													// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	if (Indices != NULL) {
		glGenBuffers(1, &IndexBufferId[ObjectId]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
	}

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)TexOffset);

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color
	glEnableVertexAttribArray(2);	// normal
	glEnableVertexAttribArray(3);

	// Disable our Vertex Buffer Object 
	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
			);
	}
}

void cleanup(void)
{
	// Cleanup VBO and shader
	for (int i = 0; i < NumObjects; i++) {
		glDeleteBuffers(1, &VertexBufferId[i]);
		glDeleteBuffers(1, &IndexBufferId[i]);
		glDeleteVertexArrays(1, &VertexArrayId[i]);
	}
	glDeleteProgram(programID);
	glDeleteProgram(pickingProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// ATTN: MODIFY AS APPROPRIATE
	if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_C:
			if (cmesh) {
				cmesh = false;
			}
			else
				cmesh = true;
			break;
		case GLFW_KEY_LEFT:
			if (selectCamera) {
				moveCameraLeft = true;
			}
			break;
		case GLFW_KEY_RIGHT:
			if (selectCamera) {
				moveCameraRight = true;
			}
			break;
		case GLFW_KEY_UP:
			if (selectCamera) {
				moveCameraUp = true;
			}
			break;
		case GLFW_KEY_DOWN:
			if (selectCamera) {
				moveCameraDown = true;
			}
			break;
		case GLFW_KEY_R:
			reset = true;
			face = false;
			cmesh = false;
			break;
		case GLFW_KEY_F:
			if (face) {
				face = false;
			}
			else
				face = true;
			break;
		case GLFW_KEY_W:
			break;
		case GLFW_KEY_0:
			smileKey = true;
			break;
		case GLFW_KEY_S:
			sKey = true;
			break;
		case GLFW_KEY_L:
			lKey = true;
			break;
		case GLFW_KEY_RIGHT_SHIFT:
			shift = true;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			shift = true;
			break;
		case GLFW_KEY_SPACE:
			raycasting = true;
			break;
		default:
			break;
		}
	}
	else if (action == GLFW_RELEASE) {
		switch (key) {
		case GLFW_KEY_LEFT:
			if (selectCamera) {
				moveCameraLeft = false;
			}
			break;
		case GLFW_KEY_RIGHT:
			if (selectCamera) {
				moveCameraRight = false;
			}
			break;
		case GLFW_KEY_UP:
			if (selectCamera) {
				moveCameraUp = false;
			}
			break;
		case GLFW_KEY_DOWN:
			if (selectCamera) {
				moveCameraDown = false;
			}
			break;
		case GLFW_KEY_R:
			if (reset) {
				reset = false;
				face = false;
				cmesh = false;
			}
			else {
				reset = true;
			}
			break;
		case GLFW_KEY_RIGHT_SHIFT:
			shift = false;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			shift = false;
			break;
		case GLFW_KEY_SPACE:
			raycasting = false;
			break;
		case GLFW_KEY_0:
				smileKey = false;
				break;
			default:
			break;
		}
	}
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pickObject();
	}
}



int main(void)
{
	int i;
	// initialize window
	int errorCode = initWindow();
	if (errorCode != 0)
		return errorCode;


	for (i = 0; i < 441; i++)
	{
		pickingColor[i] = i / 255.0f;
	}

	// initialize OpenGL pipeline
	initOpenGL();

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		if (animation) {
			phi += 0.01;
			if (phi > 360)
				phi -= 360;
		}

		//dragging
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
		{
			moveVertex();
		}


		// DRAWING POINTS
		renderScene();


	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}