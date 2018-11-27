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
