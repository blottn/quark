#include <vector>
#include "transform.h"

class SkyBox {

public:

    GLuint vao;
    GLuint shaderID;

    SkyBox(GLuint shader) {
        shaderID = shader;
        initData();
    }

    void initData() {
        GLuint VAOs[1];
        glGenVertexArrays(1,VAOs);
        vao = VAOs[0];

        glBindVertexArray(vao);
    }

    void draw() {
        glBindVertexArray(vao);
		glUseProgram(shaderID);


    }
};

class Ent {
public:
    GLuint vao;

	GLuint shaderID;

	ModelData mesh_raw;
    std::vector<Ent> * subs;
	Transform * model;

	Ent(ModelData mesh, GLuint shader, Transform * initial) {
		shaderID = shader;
		subs = new std::vector<Ent>();
		model = new Transform();

		mesh_raw = mesh;

		initData();
		model = initial;
	}

	void initData() {
		// create our vao
		GLuint VAOs[1];
		glGenVertexArrays(1, VAOs);
		vao = VAOs[0];
		glBindVertexArray(vao);

		// black magic *waves hands*
		unsigned int vp_vbo = 0;
		GLuint pos = glGetAttribLocation(shaderID, "vertex_position");
		GLuint norm = glGetAttribLocation(shaderID, "vertex_normal");
		GLuint tex = glGetAttribLocation(shaderID, "vertex_texture");

		glGenBuffers(1, &vp_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
		glBufferData(GL_ARRAY_BUFFER, mesh_raw.mPointCount * sizeof(vec3), &mesh_raw.mVertices[0], GL_STATIC_DRAW);
		unsigned int vn_vbo = 0;
		glGenBuffers(1, &vn_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
		glBufferData(GL_ARRAY_BUFFER, mesh_raw.mPointCount * sizeof(vec3), &mesh_raw.mNormals[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(pos);
		glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
		glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(norm);
		glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
		glVertexAttribPointer(norm, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	}

	Ent * addChild(Ent child) {
		subs->push_back(child);
		return this;
	}

	void bindVAO() {
		glBindVertexArray(vao);
	}

	void draw(mat4 parent, mat4 view, mat4 projection) {
		bindVAO();
		glUseProgram(shaderID);

		//Declare your uniform variables that will be used in your shader
		int matrix_location = glGetUniformLocation(shaderID, "model");
		int view_mat_location = glGetUniformLocation(shaderID, "view");
		int proj_mat_location = glGetUniformLocation(shaderID, "proj");


		// Root of the Hierarchy
		/*mat4 view = identity_mat4();
		mat4 persp_proj = perspective(45.0f, (float) glutGet(GLUT_WINDOW_WIDTH)/ (float) glutGet(GLUT_WINDOW_HEIGHT), 0.1f, 1000.0f);
		view = translate(view, vec3(0.0, 0.0, -50.0f));
*/
		// update uniforms & draw
		glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, projection.m);
		glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, (parent * model->compute()).m);
		glDrawArrays(GL_TRIANGLES, 0, mesh_raw.mPointCount);

		for(Ent child : (*subs)) {
			child.draw(parent * model->compute(), view, projection);
		}
	}
};

