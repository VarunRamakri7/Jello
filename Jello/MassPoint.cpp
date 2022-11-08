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
    this->surfacePoint = false;
}

// Set
void MassPoint::setPosition(glm::dvec3 position) {
    this->position = position;
}

void MassPoint::setFixed(bool fixed) {
    this->isFixed = fixed;
}

bool MassPoint::getFixed() {
    return this->isFixed;
}

void MassPoint::setVelocity(glm::dvec3 velocity) {
    this->velocity = velocity;
}

void MassPoint::setAcceleration(glm::dvec3 acceleration) {
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

glm::dvec3 MassPoint::getNaturalLengthV(int link)
{
    const glm::dvec3* B = this->connectedPoints.at(link)->getInitialPosition();
    return this->initialPos - *B;
}

glm::dvec3 MassPoint::getActualLengthV(int link)
{
    const glm::dvec3* B = this->connectedPoints.at(link)->getPosition();
    return this->position - *B;
}

glm::dvec3 MassPoint::getVelocityDiff(int link)
{
    const glm::dvec3* B = this->connectedPoints.at(link)->getVelocity();
    return this->velocity - *B;
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

void MassPoint::setExternalForce(glm::dvec3 force) {
    this->externalForce = force;
}

MassPoint* MassPoint::getConnection(int link) {
    return this->connectedPoints.at(link);
}

void MassPoint::addAcceleration(glm::dvec3 acc) {
    this->acceleration += acc;
}