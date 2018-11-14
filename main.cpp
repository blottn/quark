#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector>

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

#include "lib/data.h"
#include "lib/maths_funcs.cpp"
#include "lib/ent.h"
#include "lib/sky.h"

/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
#define BANANA_MESH_NAME "models/banana.dae"
#define MONKEY_MESH_NAME "models/monkeyhead_smooth.dae"

using namespace std;

// Globals

//hacky
GLuint texId;

// useful
const int width = 800;
const int height = 600;

GLuint shaderProgramID;

// Shader Functions
char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fp = fopen(shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, "shaders/simple_vert.shader", GL_VERTEX_SHADER);
	AddShader(shaderProgramID, "shaders/simple_frag.shader", GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}

Ent * root;

void display() {

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	root->draw(identity_mat4());

	glutSwapBuffers();
}

// periodic function for changing translation amts etc
void updateScene() {

	// Draw the next frame
	glutPostRedisplay();
}


void init()
{

    vector<string> * sides = new vector<string>();
    sides->push_back("models/skybox/left.jpg");
    sides->push_back("models/skybox/right.jpg");
    sides->push_back("models/skybox/top.jpg");
    sides->push_back("models/skybox/bottom.jpg");
    sides->push_back("models/skybox/front.jpg");
    sides->push_back("models/skybox/back.jpg");

    texId = loadCubemap(*sides);

	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();

	ModelData banana = load_mesh(BANANA_MESH_NAME);
	ModelData monkey = load_mesh(MONKEY_MESH_NAME);

	Transform * right = new Transform();
	right->translate = translate(right->translate, vec3(7.5f, -5.0f, 0.0f));
	right->rotate = rotate_z_deg(right->rotate, 45);

	Transform * left = new Transform();
	left->translate = translate(left->translate, vec3(-7.5f, -5.0f, 0.0f));
	left->rotate = rotate_z_deg(left->rotate, -45);

	Transform * upright = new Transform();
	upright->rotate = rotate_z_deg(upright->rotate, 90);
	upright->translate = translate(upright->translate, vec3(0.0f, -7.5f, 0.0f));


	// root
	root = new Ent(monkey, shaderProgramID, new Transform());

	// many (3)
	Ent * sub1 = new Ent(monkey, shaderProgramID, left);
	Ent * sub2 = new Ent(banana, shaderProgramID, upright);
	Ent * sub3 = new Ent(monkey, shaderProgramID, right);

	root->addChild(*sub1);
	root->addChild(*sub2);
	root->addChild(*sub3);

	Transform * bfix = new Transform();
	bfix->scale = scale(bfix->scale, vec3(0.5f, 0.5f, 0.5f));
	bfix->rotate = rotate_x_deg(bfix->rotate, 90);
	bfix->translate = translate(bfix->translate, vec3(0.0f, -1.5f, 0.0f));


	Ent * b1= new Ent(banana, shaderProgramID, bfix);
	Ent * b2 = new Ent(banana, shaderProgramID, bfix);

	sub1->addChild(*b1);
	sub3->addChild(*b2);
}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':
		root->model->translate = translate(root->model->translate, vec3(0.0f, 1.0f, 0.0f));
		break;
	case 's':
		root->model->translate = translate(root->model->translate, vec3(0.0f, -1.0f, 0.0f));
		break;
	case 'a':
		root->model->translate = translate(root->model->translate, vec3(-1.0f, 0.0f, 0.0f));
		break;
	case 'd':
		root->model->translate = translate(root->model->translate, vec3(1.0f, 0.0f, 0.0f));
		break;
	case 'z':
		root->model->translate = translate(root->model->translate, vec3(0.0f, 0.0f, -1.0f));
		break;
	case 'x':
		root->model->translate = translate(root->model->translate, vec3(0.0f, 0.0f, 1.0f));
		break;

	case 'q':
		root->model->rotate = rotate_y_deg(root->model->rotate, 5.0f);
		break;
	case 'e':
		root->model->rotate = rotate_y_deg(root->model->rotate, -5.0f);
		break;
	}
}

int main(int argc, char** argv) {
	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Models");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);
	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}
