#include <glm/glm.hpp>
using namespace glm;
// Class to describe camera position, movement
class Camera {

private:
    vec3 mPos;
    vec3 mFront;
    vec3 mLeft;
    vec3 mRight;
    vec3 mUp;

public:
    Camera(vec3 p, vec3 f, vec3 l, vec3 r, vec3 u) {
        mPos = p;
        mFront = f;
        mLeft = l;
        mRight = r;
        mUp = u;
    }

    mat4 getView() {
        return lookAt(mPos, mPos + mFront, mUp);
    }

};
