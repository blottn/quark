// Windows includes (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

#include "lib/maths_funcs.h"

/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
#define BANANA_MESH_NAME "banana.dae"
#define MONKEY_MESH_NAME "monkeyhead_smooth.dae"

using namespace std;

typedef struct
{
	size_t mPointCount = 0;
	vector<vec3> mVertices;
	vector<vec3> mNormals;
	vector<vec2> mTextureCoords;
} ModelData;


/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

ModelData load_mesh(const char* file_name) {
	ModelData modelData;

	/* Use assimp to read the model file, forcing it to be read as    */
	/* triangles. The second flag (aiProcess_PreTransformVertices) is */
	/* relevant if there are multiple meshes in the model file that   */
	/* are offset from the origin. This is pre-transform them so      */
	/* they're in the right position.                                 */
	const aiScene* scene = aiImportFile(
		file_name,
		aiProcess_Triangulate | aiProcess_PreTransformVertices
	);

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return modelData;
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
			}
			if (mesh->HasTangentsAndBitangents()) {
				/* You can extract tangents and bitangents here              */
				/* Note that you might need to make Assimp generate this     */
				/* data for you. Take a look at the flags that aiImportFile  */
				/* can take.                                                 */
			}
		}
	}

	aiReleaseImport(scene);
	return modelData;
}


const int width = 800;
const int height = 600;

GLuint shaderProgramID;

class Transform {
public:
	mat4 scale;
	mat4 rotate;
	mat4 translate;

	Transform() {
		scale = identity_mat4();
		rotate = identity_mat4();
		translate = identity_mat4();
	}

	mat4 compute() {
		return translate * (rotate * scale);
	}

	Transform * clone() {
		Transform * clone = new Transform();
		clone->scale = this->scale;
		clone->rotate = this->rotate;
		clone->translate = this->translate;
		return clone;
	}
};

class Ent {
public:
	int vao;

	GLuint loc1, loc2, loc3;

	GLuint shaderID;

	ModelData mesh_raw;
	vector<Ent> * subs;
	Transform * model;

	Ent(ModelData mesh, GLuint shader, Transform * initial) {
		shaderID = shader;
		subs = new vector<Ent>();
		model = new Transform();

		mesh_raw = mesh;

		initData();
		model = initial;
		//model->rotate = rotate_z_deg(model->rotate, 180);
	}

	void initData() {

		// create our vao
		GLuint VAOs[1];
		glGenVertexArrays(1, VAOs);
		vao = VAOs[0];
		glBindVertexArray(vao);

		// black magic *waves hands*
		unsigned int vp_vbo = 0;
		loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
		loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
		loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

		glGenBuffers(1, &vp_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
		glBufferData(GL_ARRAY_BUFFER, mesh_raw.mPointCount * sizeof(vec3), &mesh_raw.mVertices[0], GL_STATIC_DRAW);
		unsigned int vn_vbo = 0;
		glGenBuffers(1, &vn_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
		glBufferData(GL_ARRAY_BUFFER, mesh_raw.mPointCount * sizeof(vec3), &mesh_raw.mNormals[0], GL_STATIC_DRAW);


		glEnableVertexAttribArray(loc1);
		glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
		glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(loc2);
		glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
		glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	}

	Ent * addChild(Ent child) {
		subs->push_back(child);
		return this;
	}

	void bindVAO() {
		glBindVertexArray(vao);
	}

	void draw(mat4 parent) {
		bindVAO();
		glUseProgram(shaderID);

		//Declare your uniform variables that will be used in your shader
		int matrix_location = glGetUniformLocation(shaderID, "model");
		int view_mat_location = glGetUniformLocation(shaderID, "view");
		int proj_mat_location = glGetUniformLocation(shaderID, "proj");


		// Root of the Hierarchy
		mat4 view = identity_mat4();
		mat4 persp_proj = perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
		view = translate(view, vec3(0.0, 0.0, -50.0f));

		// update uniforms & draw
		glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
		glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, (parent * model->compute()).m);
		glDrawArrays(GL_TRIANGLES, 0, mesh_raw.mPointCount);

		for(Ent child : (*subs)) {
			child.draw(parent * model->compute());
		}
	}

};




GLfloat rotate_y = 0.0f;

// Shader Functions
char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

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
	AddShader(shaderProgramID, "simpleVertexShader.txt", GL_VERTEX_SHADER);
	AddShader(shaderProgramID, "simpleFragmentShader.txt", GL_FRAGMENT_SHADER);

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

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	float delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	// Rotate the model slowly around the y axis at 20 degrees per second
	rotate_y += 20.0f * delta;
	rotate_y = fmodf(rotate_y, 360.0f);

	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
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
