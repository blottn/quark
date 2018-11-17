#include <glm/glm.hpp>
using namespace glm;
// Class to describe camera position, movement
class Camera {

private:
    vec3 mPos;
    vec3 mFront;
    vec3 mRight;
    vec3 mUp;

    float pitch;
    float yaw;

public:
    Camera(vec3 p, vec3 f, vec3 r, vec3 u) {
        mPos = p;
        mFront = f;
        mRight = r;
        mUp = u;

        pitch = 0.0f;
        yaw = 0.0f;
    }

    mat4 getView() {
        return lookAt(mPos, mPos + mFront, mUp);
    }

    void move(vec3 offset) {
        mPos += offset.x * mRight;
        mPos += offset.y * mUp;
        mPos += offset.z * mFront;
    }

    void look(float xOff, float yOff) {
        xOff *= 0.1f;
        yOff *= 0.1f;

        pitch -= yOff;
        if (pitch > 89.9f) {
            pitch = 89.9f;
        }
        if (pitch < -89.9f) {
            pitch = -89.9f;
        }

        yaw += xOff;

        updateView();
    }

    void updateView() {
        vec3 front;
        front.x = cos(radians(yaw)) * cos(radians(pitch));
        front.y = sin(radians(pitch));
        front.z = sin(radians(yaw)) * cos(radians(pitch));
        mFront = normalize(front);
        // Also re-calculate the Right and Up vector
        mRight = normalize(cross(mFront, vec3(0,1,0)));  // Normalize the vectors, because th    eir length gets closer to 0 the more you look up or down which results in slower movement.
        mUp = normalize(cross(mRight, mFront));

    }


};
