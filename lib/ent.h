#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#define _USE_MATH_DEFINES

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>
#include <glm/ext.hpp>
#include "transform.h"
class SkyBox {
private:

    float verts[108] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

public:

    GLuint vao;
    GLuint shaderID;
    GLuint texture;
    SkyBox(GLuint shader, int tex) {
        texture = tex;
        shaderID = shader;
        initData();
    }

    void initData() {
        GLuint VAOs[1];
        glGenVertexArrays(1,VAOs);
        vao = VAOs[0];

        glBindVertexArray(vao);


        // generate data
        GLuint vbo_v;

        glGenBuffers(1, &vbo_v);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_v);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts) , &verts, GL_STATIC_DRAW);
        GLuint pos = glGetAttribLocation(shaderID,(const GLchar*)("pos"));
        glEnableVertexAttribArray(pos);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), NULL);
    }

    void setDepthFunc() {
        glDepthFunc(GL_LEQUAL);
    }

    void draw(glm::mat4 view, glm::mat4 projection) {
        glBindVertexArray(vao);
		glUseProgram(shaderID);
        setDepthFunc();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

		GLuint view_mat_location = glGetUniformLocation(shaderID, "view");
		GLuint proj_mat_location = glGetUniformLocation(shaderID, "projection");

        glm::mat4 new_view = glm::mat4(glm::mat3(view));

		glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(new_view));

        glDrawArrays(GL_TRIANGLES,0,sizeof(verts));
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
		glBufferData(GL_ARRAY_BUFFER, mesh_raw.mPointCount * sizeof(glm::vec3), &mesh_raw.mVertices[0], GL_STATIC_DRAW);
		unsigned int vn_vbo = 0;
		glGenBuffers(1, &vn_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
		glBufferData(GL_ARRAY_BUFFER, mesh_raw.mPointCount * sizeof(glm::vec3), &mesh_raw.mNormals[0], GL_STATIC_DRAW);

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

	void draw(glm::mat4 parent, glm::mat4 view, glm::mat4 projection) {
		bindVAO();
		glUseProgram(shaderID);
        glDepthFunc(GL_GREATER);
		//Declare your uniform variables that will be used in your shader
		int matrix_location = glGetUniformLocation(shaderID, "model");
		int view_mat_location = glGetUniformLocation(shaderID, "view");
		int proj_mat_location = glGetUniformLocation(shaderID, "proj");


		// update uniforms & draw
		glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(view));

		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, glm::value_ptr((parent * model->compute())));
		glDrawArrays(GL_TRIANGLES, 0, mesh_raw.mPointCount);

		for(Ent child : (*subs)) {
			child.draw(parent * model->compute(), view, projection);
		}
	}
};

class Plane {
private:
    int vao;

public:
    int id;
    Transform * transform;

    float verts[18] = {
        100,0,100,
        -100,0,100,
        100,0,-100,

        -100,0,-100,
        -100,0,100,
        100,0,-100
    };

    float norms[18] = {
        0,1,0,
        0,1,0,
        0,1,0,

        0,1,0,
        0,1,0,
        0,1,0
    };

    Plane(int shaderId, Transform * transf) {
        id = shaderId;
        transform = transf;

        initData();
    }

    void initData() {



        GLuint VAOs[1];
        glGenVertexArrays(1, VAOs);
        vao = VAOs[0];
        glBindVertexArray(vao);

        unsigned int vp_vbo = 0;
        GLuint pos = glGetAttribLocation(id, "vertex_position");
        GLuint norm = glGetAttribLocation(id, "vertex_normal");

        glGenBuffers(1, &vp_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
        glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), &verts, GL_STATIC_DRAW);

        unsigned int vn_vbo = 0;
        glGenBuffers(1, &vn_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
        glBufferData(GL_ARRAY_BUFFER, 18*sizeof(float), &norms, GL_STATIC_DRAW);

        glEnableVertexAttribArray(pos);
        glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
        glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(norm);
        glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
        glVertexAttribPointer(norm, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    }

    void draw(glm::mat4 view, glm::mat4 proj) {
        glBindVertexArray(vao);
        glDepthFunc(GL_LESS);
        glUseProgram(id);

	    int matrix_location = glGetUniformLocation(id, "model");
		int view_mat_location = glGetUniformLocation(id, "view");
		int proj_mat_location = glGetUniformLocation(id, "proj");

		// update uniforms & draw
		glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, glm::value_ptr(proj));
		glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, glm::value_ptr(transform->compute()));


        glDrawArrays(GL_TRIANGLES,0,sizeof(verts));
    }
};
