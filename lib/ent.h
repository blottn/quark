#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#define _USE_MATH_DEFINES

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>
#include "glm/ext.hpp"
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
        glDepthFunc(GL_LESS);
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

class Sphere {
private:
    GLuint vao;
    GLuint ebo;

    float * verts;
    int vCount;

public:
    GLuint id;
    
    glm::vec3 c; 
    float r;
    Transform * transform;

    float cCount;
    float aCount;

    Sphere(int shader, glm::vec3 centre, float rad, int crosses, int arms, Transform * transf) {
        id = shader;
        c = centre;
        r = rad;
        
        cCount = crosses;
        aCount = arms;

        vCount = 3 * (crosses * (2 * arms) + 2);
        
        transform = transf;
        initData();
    }

    void initData() {

        // generate crosses and arms
        float data[vCount];
    
        // both ends
        data[0] = c.x;
        data[1] = c.y - r;
        data[2] = c.z;

        int dataInd = 3;
        for (float i = 0 ; i < cCount ; i++) {
            // convert i into a y value
            //step is rad*2 / cCount+1
            float y = c.y - r + (i+1)*( (float) (r*2) / (float) (cCount+1));

            //get radius at this height
            float yR = sqrt(pow(r,2) - pow(y,2));
            for (int j = 0; j < aCount ; j++ ) {

                float divisionAmt = ((float)3.1415926535) / ((float) aCount + 1.0);
                float x = sin(divisionAmt * (j + 1) ) * yR;
                float z = cos(divisionAmt * (j + 1) ) * yR;
                data[dataInd] = (float) (c.x + x);
                data[dataInd + 1] = y;
                data[dataInd + 2] = (float) (c.z + z);
                int semiOff = 3*aCount;
                data[dataInd + semiOff ] = (float) (c.x - x);
                data[dataInd + semiOff + 1] = y;
                data[dataInd + semiOff + 2] = (float) (c.z - z);
                std::cout << "writing to: " << std::endl << dataInd << " -> " << dataInd + 2 << std::endl << dataInd + semiOff << " -> " << dataInd + semiOff + 2 << std::endl;
               //TODO why is this producing minint sometimes and only when not printed
                dataInd += 3;
            }
            dataInd += 3*aCount;
        }

        data[vCount - 3] = c.x;
        data[vCount - 2] = c.y + r;
        data[vCount - 1] = c.z;


        

        verts = &data[0];

        for (int i = 0 ; i < vCount ; i++) {
            std::cout << i << ": " << verts[i] << std::endl;
        }


        GLuint VAOs[1];
        glGenVertexArrays(1,VAOs);
        vao = VAOs[0];

        glBindVertexArray(vao);


        // generate data

        glGenBuffers(1, &ebo);
 
        // top and bottom, middles
        int indexCount =  3*(4*(int)aCount + ((int)cCount - 2 + 1) * 4 * (int)aCount);
        int indexes[indexCount];
        //bottom
        for (int i = 0; i < aCount*2 ; i++) {
            indexes[i*3] = 0;
            indexes[i*3 + 1] = i+1;
            indexes[i*3 + 2] = ((i+1) % (2 * (int) aCount)) + 1;
        }

        //aCount = 2
        //cCount = 2

        int trisPerCap = 2*aCount;
        int trisPerRow = 2*2*aCount;

        //middles
        for (int i = 0 ; i < cCount -1; i++) {     //over rows
            
            int rowStartIndex = 1 + i*aCount*2;
            int nextRowStartIndex = 1 + (i+1)*aCount*2;
            
            // iterate over index offset from row start
            for (int j = 0; j < 3 * aCount*2*2; j += 6) {
                int first = rowStartIndex + j / 6;
                int right = (((first + 1) - rowStartIndex)%4)+rowStartIndex;
                int up = first + aCount*2;
                int upRight = (((up + 1) - nextRowStartIndex)%4) + nextRowStartIndex;

                indexes[trisPerCap*3 + (int)aCount*2*i + j + 0] = first;
                indexes[trisPerCap*3 + (int)aCount*2*i + j + 1] = right;
                indexes[trisPerCap*3 + (int)aCount*2*i + j + 2] = upRight;
                //      bottomcap      previous rows 
                indexes[trisPerCap*3 + (int)aCount*2*i + j + 3] = first;
                indexes[trisPerCap*3 + (int)aCount*2*i + j + 4] = up;
                indexes[trisPerCap*3 + (int)aCount*2*i + j + 5] = upRight;
            }
        }

        int maxInd = 2 + cCount * aCount * 2 - 1;//9
        int lowTop = maxInd - 2* aCount;
        for (int i = 0; i < 3*2*aCount; i+=3) {
            indexes[indexCount - 2*3 * (int)aCount + i] = lowTop + i/3;
            indexes[indexCount - 2*3 * (int)aCount + i + 1] = ((lowTop + i/3 + 1 -lowTop) % 4) +lowTop;
            indexes[indexCount - 2*3 * (int)aCount + i + 2] = maxInd;
        }


        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW); 

        GLuint vbo_v;
        glGenBuffers(1, &vbo_v);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_v);
		glBufferData(GL_ARRAY_BUFFER, sizeof(data) , verts, GL_STATIC_DRAW); // for some reason only works with sizeof(dataa), (36 when verts gives 32)
        
        
        GLuint pos = glGetAttribLocation(id,(const GLchar*)("vertex_position"));
        glEnableVertexAttribArray(pos);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), NULL);
    }
    
    void bindVAO() {
        glBindVertexArray(vao);
    }
    
    void useShader() {
        glUseProgram(id);
    }

    void draw(glm::mat4 view, glm::mat4 projection) {
        bindVAO();
        useShader();
        glDepthFunc(GL_LESS);

        int matrix_location = glGetUniformLocation(id, "model");
        int view_mat_location = glGetUniformLocation(id, "view");
        int proj_mat_location = glGetUniformLocation(id, "proj");


        // update uniforms & draw
        glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, glm::value_ptr(projection)    );
        glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(matrix_location, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glDrawElements(GL_TRIANGLE_STRIP, 2*vCount, GL_UNSIGNED_INT, 0);
    }
};
