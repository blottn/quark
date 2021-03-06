#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector>

// OpenGL includes
#include <GL/glew.h>
#include <GLUT/glut.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/rotate_vector.hpp>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

#include "lib/sky.h"
#include "lib/sphere.h"
#include "lib/texture.h"
#include "lib/camera.h"
#include "lib/particle.h"

using namespace std;

// Globals

// useful
const int width = 1600;
const int height = 800;

const int SPHERE_RES = 100;

const float G = 0.00006;  //atll do for accuracy

const int middleX = ((float) width ) / 2.0f;
const int middleY = ((float) height ) / 2.0f;

SkyBox * sky;
ParticleEffect * sunParticles;
PhysicsSphere * physicsSphere;

Camera * camera;

// dont immeadiately lock in the mouse
int warp = 0;

//TODO remove in favour of camera
Transform * view = new Transform();
glm::mat4 projection = glm::mat4(1.0f);

Sphere * sun;

// Shader Functions
char* readShaderSource(const char* shaderFile) {
	FILE* fp;
    int errorno;
	fp = fopen(shaderFile, "rb");
	if (fp == NULL) {return NULL; }
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
    
	GLuint shaderProgramID = glCreateProgram();
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
		// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}

void display() {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    sky->draw(camera->getView(), projection);
    sun->draw(mat4(1.0f),camera->getView(), projection, camera->mPos);
    physicsSphere->draw(mat4(1.0f), camera->getView(), projection, camera->mPos);
    sunParticles->draw(camera->getView(), projection);
    if (warp)
        glutWarpPointer(middleX, middleY);
	glutSwapBuffers();
}



vec3 getPull(Sphere * obj) {
    glm::vec3 r_dir = glm::vec3(obj->transform->translate * vec4(0.0f,0.0f,0.0f,1.0f) - vec4(physicsSphere->pos, 1.0f));
    float r = glm::length(r_dir);
    r = r * r;  //r^2
    float mag = ((G * obj->mass * 1.0f) / r);
    return glm::normalize(r_dir) * ((G * obj->mass * 1.0f) / r);
}


void updateScene() {
    sun->updateChildren();
    sunParticles->update();


    vec3 accel = getPull(sun);
    physicsSphere->update(accel);
    if (warp)
        camera->mPos = physicsSphere->pos + vec3(0.0,0.0,-0.20);
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

    int texId = loadCubemap(*sides);

    sky = new SkyBox(skyShader, texId);

}

void initPlanets() {
    GLuint sphereShader = CompileShaders("shaders/sphere_vert.shader","shaders/sphere_frag.shader");

    GLuint sunTex = load("models/sun.jpg",0);
    GLuint earthTex = load("models/earth.jpg",0);
    GLuint moonTex = load("models/moon.jpg",0);

    Transform * sphereTransform = new Transform();
    sphereTransform->scale = scale(sphereTransform->scale, vec3(0.2,0.2,0.2));
    sun = new Sphere(sphereShader, vec3(0,0,0),10, SPHERE_RES, SPHERE_RES, sphereTransform, sunTex, 1, 1);

    Transform * planetTransform = new Transform();
    planetTransform->scale = scale(planetTransform->scale, vec3(0.5,0.5,0.5));
    planetTransform->translate = translate(planetTransform->translate, vec3(200,0,0));

    Transform * moonTransform = new Transform();
    moonTransform->scale = scale(moonTransform->scale, vec3(0.5,0.5,0.5));
    moonTransform->translate = translate(moonTransform->translate, vec3(10,0,0));

    Sphere * earth = new Sphere(sphereShader, vec3(0,0,0), 10, SPHERE_RES, SPHERE_RES, planetTransform, earthTex, 0, 0.5);
    Sphere * moon = new Sphere(sphereShader, vec3(0,0,0), 10, SPHERE_RES, SPHERE_RES, planetTransform, moonTex, 0, 0.02);
    moon->ORBIT_SPEED = 0.04f;
    sun->addChild(*earth);
    earth->addChild(*moon);
}

void initParticles() {

    GLuint particleShader = CompileShaders("shaders/particle_vert.shader", "shaders/particle_frag.shader");
    sunParticles = new ParticleEffect(vec3(0,0,0), particleShader, 5);  //magic numbers!
    Transform * partTra = new Transform();
}

void initPhysics() {
    GLuint physicsSphereShader = CompileShaders("shaders/sphere_vert.shader","shaders/sphere_frag.shader");
    GLuint rockTex = load("models/rock.jpg",0);
    physicsSphere = new PhysicsSphere(physicsSphereShader, SPHERE_RES, SPHERE_RES, rockTex);
}

void init() {
    // view and projection init
    projection = glm::perspective(glm::radians(60.0f), (float) glutGet(GLUT_WINDOW_WIDTH)/ (float) glutGet(    GLUT_WINDOW_HEIGHT), 0.1f, 1000.0f);
    view->translate = glm::translate(view->translate, glm::vec3(0.0f, 0.0f, -50.0f));

    camera = new Camera(
            glm::vec3(0,0,10),
            glm::vec3(0,0,-1),
            glm::vec3(1,0,0),
            glm::vec3(0,1,0));

    initSkybox();

    initPlanets();
    initParticles();
    initPhysics();
}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
    const int lookspeed = 20;
	switch (key) {
    case 'a': //left
        camera->move(vec3(-0.1,0,0));
        break;
    case 'd':
        camera->move(vec3(0.1,0,0));
        break;
    case 'r':   //up
        camera->move(vec3(0,0.1,0));
        break;
    case 'e':
        camera->move(vec3(0,-0.1,0));
        break;
    case 'w':       //forwards
        camera->move(vec3(0,0,0.1));
        break;
    case 's':
        camera->move(vec3(0,0,-0.1));
        break;
    case ' ':
        if (warp)
            warp = 0;
        else
            warp = 1;
        break;
    case 'y':
        camera->look(-lookspeed,0);
        break;
    case 'u':
        camera->look(lookspeed,0);
        break;
    case 'f':
        camera->look(0,-lookspeed);
        break;
    case 'v':
        camera->look(0,lookspeed);
        break;
    case 'q':   //exit
        exit(0);
        break;
	}
}

void mouseMove(int x, int y) {
    if (warp)
        camera->look(x - middleX, y-middleY);
}

int main(int argc, char** argv) {
	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DEPTH |  GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Quark");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);

    glutKeyboardFunc(keypress);
    glutPassiveMotionFunc(mouseMove);
 //   glutSetCursor(GLUT_CURSOR_NONE);

    glewExperimental = GL_TRUE;
	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

	// Set up your objects and shaders
	init();

	// Begin infinite event loop
	glutMainLoop();
	return 0;
}
