#include "Camera.h"

Camera::Camera() {}

void Camera::setPosition(glm::vec3 position) {
    this->position = position;
}

void Camera::setLookAt(glm::vec3 lookAt) {
    this->lookAt = lookAt;
}

glm::vec3 Camera::getPosition() {
    return position;
}

glm::vec3 Camera::getLookAt() {
    return lookAt;
}

glm::mat4 Camera::getPV() {
    glm::vec3 cameraPos = this->getPosition();
    glm::vec3 cameraLookAt = this->getLookAt();
    return this->P * glm::lookAt(cameraPos, cameraLookAt, yUp);
}