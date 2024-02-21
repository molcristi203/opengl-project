#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraFrontDirection, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraFrontDirection = cameraFrontDirection;
        this->cameraUpDirection = cameraUp;
        this->cameraRightDirection = glm::normalize(glm::cross(cameraUp, cameraFrontDirection));
        this->cameraLookDirection = cameraFrontDirection;
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(this->cameraPosition, this->cameraPosition + this->cameraLookDirection, this->cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        switch (direction) {
            case MOVE_FORWARD:
                cameraPosition += speed * cameraFrontDirection;
                break;
            case MOVE_BACKWARD:
                cameraPosition -= speed * cameraFrontDirection;
                break;
            case MOVE_LEFT:
                cameraPosition += speed * cameraRightDirection;
                break;
            case MOVE_RIGHT:
                cameraPosition -= speed * cameraRightDirection;
                break;
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 front;
        glm::vec3 look;
        
        front.x = cos(glm::radians(yaw));
        front.y = 0.0;
        front.z = sin(glm::radians(yaw));

        look.x = front.x * cos(glm::radians(pitch));
        look.y = sin(glm::radians(pitch));
        look.z = front.z * cos(glm::radians(pitch));

        cameraFrontDirection = glm::normalize(front);
        cameraLookDirection = glm::normalize(look);

        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraUpDirection, this->cameraFrontDirection));
    }
    glm::vec3 Camera::getPosition()
    {
        return this->cameraPosition;
    }
    void Camera::setPosition(glm::vec3 cameraPosition)
    {
        this->cameraPosition = cameraPosition;
    }

    glm::vec3 Camera::getFrontDirection()
    {
        return this->cameraFrontDirection;
    }

    glm::vec3 Camera::getRightDirection()
    {
        return this->cameraRightDirection;
    }

}
