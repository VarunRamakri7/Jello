#include "MassPoint.h"

#include <iostream>

MassPoint::MassPoint(glm::vec3 position, bool isSurfacePoint){
    this->position = position;
    this->initialPos = position;
    this->surfacePoint = isSurfacePoint;
}


// Set
void MassPoint::setPosition(glm::vec3 position) {
    this->position = position;
}

void MassPoint::setFixed(bool fixed) {
    this->isFixed = fixed;
}

bool MassPoint::getFixed() {
    return this->isFixed;
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
    this->connectedPoints.push_back(m);
}

void MassPoint::clearAllConnections() {
    // TODO is ever used? 
    this->connectedPoints.clear();
}

const int MassPoint::getConnectionCount() {
    return this->connectedPoints.size();
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

glm::vec3 MassPoint::getNaturalLengthV(int link)
{
    const glm::vec3* B = this->connectedPoints.at(link)->getInitialPosition();
    return this->initialPos - *B;
}

glm::vec3 MassPoint::getActualLengthV(int link)
{
    const glm::vec3* B = this->connectedPoints.at(link)->getPosition();
    return this->position - *B;
}

glm::vec3 MassPoint::getVelocityDiff(int link)
{
    const glm::vec3* B = this->connectedPoints.at(link)->getVelocity();
    return this->velocity - *B;
}

const glm::vec3* MassPoint::getInitialPosition()
{
    return &this->initialPos;
}

const glm::vec3* MassPoint::getExternalForce()
{
    return &this->externalForce;
}

void MassPoint::setExternalForce(glm::vec3 force) {
    this->externalForce = force;
}