

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

