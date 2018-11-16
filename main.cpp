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
#include "lib/ent.h"
#include "lib/sky.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

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

Ent * root;
SkyBox * sky;

Transform * view = new Transform();
glm::mat4 projection = glm::mat4(1.0f);

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

GLuint CompileShaders(string vertex_file, string fragment_file)
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
	AddShader(shaderProgramID, vertex_file.c_str(), GL_VERTEX_SHADER);
	AddShader(shaderProgramID, fragment_file.c_str(), GL_FRAGMENT_SHADER);

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


void dump_view() {
    glm::mat4 te = glm::mat4(1.0f);
    cout << glm::to_string(te) << endl;
//    cout << glm::to_string(view->compute()) << endl;
}

int t= 0;

void display() {

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);


    glm::mat4 temp_v = glm::mat4(1.0f);

    temp_v = glm::rotate(temp_v, glm::radians(65.0f), glm::vec3(1,0,0));
    temp_v = glm::translate(temp_v, glm::vec3(0,0,-20));
//    root->draw(glm::mat4(1.0f), temp_v, projection);
	root->draw(glm::mat4(1.0f), view->compute(), projection);


    if (t == 0) {
        t = 1;
        dump_view();
    }

//    sky->draw(temp_v, projection);
    sky->draw(view->compute(), projection);

	glutSwapBuffers();
}

// periodic function for changing translation amts etc
void updateScene() {
	// Draw the next frame
	glutPostRedisplay();
}

void initSkybox() {

    GLuint skyShader = CompileShaders("shaders/sky_vert.shader","shaders/sky_frag.shader");

    vector<string> * sides = new vector<string>();
    sides->push_back("models/skybox/right.jpg");
    sides->push_back("models/skybox/left.jpg");
    sides->push_back("models/skybox/top.jpg");
    sides->push_back("models/skybox/bottom.jpg");
    sides->push_back("models/skybox/front.jpg");
    sides->push_back("models/skybox/back.jpg");

    texId = loadCubemap(*sides);

    sky = new SkyBox(skyShader, texId);

}


void init()
{

    // Set up the shaders
	GLuint shaderProgramID = CompileShaders("shaders/simple_vert.shader","shaders/simple_frag.shader");

	ModelData monkey = load_mesh(MONKEY_MESH_NAME);

	Transform * right = new Transform();
	right->translate = glm::translate(right->translate, glm::vec3(7.5f, -5.0f, 0.0f));
	right->rotate = glm::rotate(right->rotate, glm::radians(45.0f), glm::vec3(0,0,1));

	Transform * left = new Transform();
	left->translate = glm::translate(left->translate, glm::vec3(-7.5f, -5.0f, 0.0f));
	left->rotate = glm::rotate(left->rotate, glm::radians(-45.0f), glm::vec3(0,0,1));


	// root
	root = new Ent(monkey, shaderProgramID, new Transform());

	Ent * sub1 = new Ent(monkey, shaderProgramID, left);

	root->addChild(*sub1);

    // view and projection init
    projection = glm::perspective(glm::radians(90.0f), (float) glutGet(GLUT_WINDOW_WIDTH)/ (float) glutGet(    GLUT_WINDOW_HEIGHT), 0.1f, 1000.0f);
    view->translate = glm::translate(view->translate, glm::vec3(0.0f, 0.0f, -50.0f));

    initSkybox();
}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':
		root->model->translate = glm::translate(root->model->translate, glm::vec3(0.0f, 1.0f, 0.0f));
		break;
	case 's':
		root->model->translate = glm::translate(root->model->translate, glm::vec3(0.0f, -1.0f, 0.0f));
		break;
	case 'a':
		root->model->translate = glm::translate(root->model->translate, glm::vec3(-1.0f, 0.0f, 0.0f));
		break;
	case 'd':
		root->model->translate = glm::translate(root->model->translate, glm::vec3(1.0f, 0.0f, 0.0f));
		break;
	case 'z':
		view->translate = glm::translate(view->translate, glm::vec3(0.0f, 0.0f, 1.0f));
		break;
	case 'x':
		view->translate = glm::translate(view->translate, glm::vec3(0.0f, 0.0f, -1.0f));
		break;

	case 'q':
		view->rotate = glm::rotate(view->rotate, glm::radians(5.0f), glm::vec3(0,1,0));
		break;
	case 'e':
		view->rotate = glm::rotate(view->rotate, glm::radians(-5.0f),glm::vec3(0,1,0));
		break;

	case 'Q':
		view->rotate = glm::rotate(view->rotate, glm::radians(5.0f), glm::vec3(1,0,0));
		break;
	case 'E':
		view->rotate = glm::rotate(view->rotate, glm::radians(-5.0f),glm::vec3(1,0,0));
        dump_view();
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
