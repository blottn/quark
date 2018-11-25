#define GLM_ENABLE_EXPERIMENTAL
#define _USE_MATH_DEFINES

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>
#include <glm/ext.hpp>
#include <vector>

using namespace std;

const float ORBIT_SPEED = 0.03f;

class Sphere {
private:
    GLuint vao;
    GLuint ebo;

    vector<Sphere> * subs;

    float orbitDeg = 0;

    float * verts;
    int vCount;
   

public:
    GLuint id;
    GLuint tex;

    glm::vec3 c; 
    float r;
    
    Transform * transform;

    int cCount;
    int aCount;

    Sphere(int shader, glm::vec3 centre, float rad, int crosses, int arms, Transform * transf, GLuint texture) {
        id = shader;
        c = centre;
        r = rad;
        tex = texture;

        subs = new vector<Sphere>();

        cCount = crosses;
        aCount = arms;

        vCount = 3 * (crosses * (2 * arms) + 2);
        
        this->transform = transf;
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
                dataInd += 3;
            }
            dataInd += 3*aCount;
        }

        data[vCount - 3] = c.x;
        data[vCount - 2] = c.y + r;
        data[vCount - 1] = c.z;
        
        verts = &data[0];
        

        GLuint VAOs[1];
        glGenVertexArrays(1,VAOs);
        vao = VAOs[0];

        glBindVertexArray(vao);


        // generate data

        glGenBuffers(1, &ebo);
 
        // top and bottom, middles
        // number of indexes = 3 * number of triangles
        // number of triangles = (aCount * 2)*2 + faceCount*2
        // number of faces = (aCount * 2) * (cCount-1)
        int faceCount = aCount * 2 * (cCount - 1);
        int triCount = faceCount *2  + (aCount *4);
        int indexCount = 3 * triCount;

        
        int indexes[indexCount];
        //bottom
        for (int i = 0; i < aCount*2 ; i++) {
            indexes[i*3] = 0;
            indexes[i*3 + 1] = i+1;
            indexes[i*3 + 2] = ((i+1) % (2 * aCount)) + 1;
        }

        int trisPerCap = 2*aCount;
        int trisPerRow = 2*2*aCount;

        //middles
        for (int i = 0 ; i < cCount -1; i++) {     //over rows
            int rowStartIndex = 1 + i*aCount*2;
            int nextRowStartIndex = 1 + (i+1)*aCount*2;
            // iterate over index offset from row start
            for (int j = 0; j < 3 * aCount*2*2; j += 6) {
                int first = rowStartIndex + j / 6;
                int right = (((first + 1) - rowStartIndex)% ((aCount)*2))+rowStartIndex;
                int up = first + aCount*2;
                int upRight = (((up + 1) - nextRowStartIndex)% (( aCount)*2)) + nextRowStartIndex;

                indexes[trisPerCap*3 + aCount*2*2*3*i + j + 0] = first;
                indexes[trisPerCap*3 + aCount*2*2*3*i + j + 1] = right;
                indexes[trisPerCap*3 + aCount*2*2*3*i + j + 2] = upRight;
                //      bottomcap      previous rows 
                indexes[trisPerCap*3 + aCount*2*2*3*i + j + 3] = first;
                indexes[trisPerCap*3 + aCount*2*2*3*i + j + 4] = up;
                indexes[trisPerCap*3 + aCount*2*2*3*i + j + 5] = upRight;
            }
        }

        int maxInd = 2 + cCount * aCount * 2 - 1;
        int lowTop = maxInd - 2* aCount;
        for (int i = 0; i < 3*2*aCount; i+=3) {
            indexes[indexCount - 2*3 * aCount + i] = lowTop + i/3;
            indexes[indexCount - 2*3 * aCount + i + 1] = ((lowTop + i/3 + 1 -lowTop) % (( aCount) *2)) +lowTop;
            indexes[indexCount - 2*3 * aCount + i + 2] = maxInd;
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
 
    void addChild(Sphere sphere) {
        subs->push_back(sphere);
    }

    void bindVAO() {
        glBindVertexArray(vao);
    }
    
    void useShader() {
        glUseProgram(id);
    }

    void draw(glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
        bindVAO();
        useShader();
        glDepthFunc(GL_LESS);

        int model_mat_location = glGetUniformLocation(id, "model");
        int view_mat_location = glGetUniformLocation(id, "view");
        int proj_mat_location = glGetUniformLocation(id, "proj");

        // update uniforms & draw
        glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, glm::value_ptr(projection)    );
        glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, glm::value_ptr(model * transform->compute()));

        glBindTexture(GL_TEXTURE_2D, tex);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glDrawElements(GL_TRIANGLES, 3*vCount, GL_UNSIGNED_INT, 0);

        for (Sphere child : (*subs) ) {
            child.draw(model * transform->compute(), view, projection);
        }
    }
    void updateChildren() {
        for (Sphere child : (*subs) ) {
            child.update();
        }
    }

    void update() {
        this->transform->orbital = glm::rotate(this->transform->orbital, glm::radians(ORBIT_SPEED), glm::vec3(0,1,0));
        updateChildren();
    }
};
