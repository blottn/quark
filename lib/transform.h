

class Transform {
public:
    glm::mat4 scale;
    glm::mat4 rotate;
    glm::mat4 translate;

    Transform() {
        scale = glm::mat4(1.0f);
        rotate = glm::mat4(1.0f);
        translate = glm::mat4(1.0f);
    }

    glm::mat4 compute() {
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

