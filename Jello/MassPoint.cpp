#include "MassPoint.h"

MassPoint::MassPoint(glm::vec3 position, bool isSurfacePoint){
    this->position = position;
    this->initialPos = position;
    this->surfacePoint = isSurfacePoint;
}


// Set
void MassPoint::setPosition(glm::vec3 position) {
    this->position = position;
}

void MassPoint::setVelocity(glm::vec3 velocity) {
    this->velocity = velocity;
}

void MassPoint::setAcceleration(glm::vec3 acceleration) {
    this->acceleration = acceleration;
}

bool MassPoint::isSurfacePoint() {
    return this->surfacePoint;
}

// Process
void MassPoint::addConnection(MassPoint* m) {
    connectedPoints.push_back(m);
}

void MassPoint::clearAllConnections() {

}

const glm::vec3* MassPoint::getPosition() {
    return &this->position;
}

const glm::vec3* MassPoint::getVelocity() {
    return &this->velocity;
}
const glm::vec3* MassPoint::getAcceleration() {
    return &this->acceleration;
}