

class Transform {
public:
    glm::mat4 scale;
    glm::mat4 rotate;
    glm::mat4 translate;
    glm::mat4 orbital;

    Transform() {
        scale = glm::mat4(1.0f);
        rotate = glm::mat4(1.0f);
        translate = glm::mat4(1.0f);
        orbital = glm::mat4(1.0f);
    }

    glm::mat4 compute() {
        glm::mat4 out = orbital * (translate * (rotate*(scale*glm::mat4(1.0f))));
        /*out = out * scale;
        out = out * rotate;
        out = out * translate;
        out = out * orbital;*/

        return out;
    }

    Transform * clone() {
        Transform * clone = new Transform();
        clone->scale = this->scale;
        clone->rotate = this->rotate;
        clone->translate = this->translate;
        clone->orbital = this->orbital;
        return clone;
    }
};

