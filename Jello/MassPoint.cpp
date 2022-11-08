#include "MassPoint.h"

#include <iostream>

MassPoint::MassPoint(glm::dvec3 position, bool isSurfacePoint){
    this->position = position;
    this->initialPos = position;
    this->surfacePoint = isSurfacePoint;
}

MassPoint::MassPoint(glm::dvec3 position, glm::dvec3 velocity) {
    this->position = position;
    this->initialPos = position;
    this->velocity = velocity;
}

// Set
void MassPoint::setPosition(glm::dvec3 position) {
    this->position = position;
}

void MassPoint::setFixed(bool fixed) {
    this->fixed = fixed;
}

void MassPoint::setVelocity(glm::dvec3 velocity) {
    this->velocity = velocity;
}

void MassPoint::setAcceleration(glm::dvec3 acceleration) {
    this->acceleration = acceleration;
}

void MassPoint::setExternalForce(glm::dvec3 force) {
    this->externalForce = force;
}

void MassPoint::addAcceleration(glm::dvec3 acc) {
    this->acceleration += acc;
}


// Process
void MassPoint::addConnection(MassPoint* m) {
    this->connectedPoints.push_back(m);
}

void MassPoint::clearAllConnections() {
    // TODO is ever used? 
    this->connectedPoints.clear();
}

// get

bool MassPoint::isSurfacePoint() {
    return this->surfacePoint;
}

bool MassPoint::isFixed() {
    return this->fixed;
}

int MassPoint::getConnectionCount() {
    return this->connectedPoints.size();
}

glm::dvec3* MassPoint::getPosition() {
    return &this->position;
}

glm::dvec3* MassPoint::getVelocity() {
    return &this->velocity;
}
glm::dvec3* MassPoint::getAcceleration() {
    return &this->acceleration;
}

// to do make pointer to constant value 
glm::dvec3* MassPoint::getInitialPosition()
{
    return &this->initialPos;
}

glm::dvec3* MassPoint::getExternalForce()
{
    return &this->externalForce;
}

MassPoint* MassPoint::getConnection(int link) {
    return this->connectedPoints.at(link);
}

