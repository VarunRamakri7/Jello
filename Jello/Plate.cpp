#include "Plate.h"
#include <iostream>

Plate::Plate(glm::vec3 center, double size, GLuint debugShader) {
    this->size = size;
    double half = size * 0.5;
    glm::vec3 pointA = center + glm::vec3(-half, 0, -half); // top left
    glm::vec3 pointB = center + glm::vec3(half, 0, -half); // top right
    glm::vec3 pointC = center + glm::vec3(-half, 0, half); // bottom left
    glm::vec3 pointD = center + glm::vec3(half, 0, half); // bottom right
    platePlane = new Plane(pointA, pointB, pointC, pointD, debugShader);
    platePlane->setPosition(center);
}

void Plate::render(GLuint modelParameter) {
    platePlane->render(modelParameter);
}

void Plate::setPosition(glm::vec3 position, double timeStep) {
    double half = this->size * 0.5;

    //glm::dvec3 posOffset = position - this->position;
    glm::dvec3 posOffset = position - platePlane->getPosition();

    // move constraint points
    for (const auto& p : this->constraintPoints) {
        // this keeps on adding ...  TODO cap this 
        p->setPosition(*p->getPosition() + posOffset);
        // change in position over change in time
        // should time be a global variable 
        glm::dvec3 vel = posOffset / timeStep;

        p->setVelocity(vel);
    }

    //this->position = position;
    platePlane->setPosition(position);
}


void Plate::setConstraintPoints(std::vector <MassPoint*> points) {
    this->constraintPoints = points;
}
