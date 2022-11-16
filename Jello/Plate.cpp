#include "Plate.h"
#include <iostream>

Plate::Plate(glm::vec3 center, double size) {
    this->size = size;
    double half = size * 0.5;
    glm::vec3 pointA = center + glm::vec3(-half, 0, -half); // top left
    glm::vec3 pointB = center + glm::vec3(half, 0, -half); // top right
    glm::vec3 pointC = center + glm::vec3(-half, 0, half); // bottom left
    glm::vec3 pointD = center + glm::vec3(half, 0, half); // bottom right
    platePlane = new Plane(pointA, pointB, pointC, pointD);
    this->position = center;
}

void Plate::render(GLuint modelParameter) {
    // no need  to pass to shader cos we need  calculate collision 
    

    //platePlane->render(modelParameter);

    // TODO because this one moves we do this here 
    glBindVertexArray(platePlane->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, platePlane->VBO);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // TODO need to send in the actual points, we dont pass in the model cos we use it to calculate collision here any ways 

    // send to GPU
    // rendering as triangle strip
    platePlane->data.push_back(platePlane->pointA.x);
    platePlane->data.push_back(platePlane->pointA.y);
    platePlane->data.push_back(platePlane->pointA.z);

    platePlane->data.push_back(platePlane->pointC.x);
    platePlane->data.push_back(platePlane->pointC.y);
    platePlane->data.push_back(platePlane->pointC.z);

    platePlane->data.push_back(platePlane->pointB.x);
    platePlane->data.push_back(platePlane->pointB.y);
    platePlane->data.push_back(platePlane->pointB.z);

    platePlane->data.push_back(platePlane->pointD.x);
    platePlane->data.push_back(platePlane->pointD.y);
    platePlane->data.push_back(platePlane->pointD.z);

    platePlane->dataSize = platePlane->data.size();

    glBufferData(GL_ARRAY_BUFFER, platePlane->dataSize * sizeof(GLfloat), platePlane->data.data(), GL_DYNAMIC_DRAW);

    platePlane->data.clear();


    // enable point size
    // 
    //glEnable(GL_PROGRAM_POINT_SIZE);
    glPointSize(5);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, platePlane->dataSize / 3); // TODO Really? can i divide 3 again? 
    //glDisable(GL_PROGRAM_POINT_SIZE)


    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Plate::offsetPosition(glm::dvec3 offset, double timeStep) {
    // TRANSLATE THE ACTUAL POINTS 
    //this->position += offset;

    //std::cout << this->position.x << ", " << this->position.y << ", " << this->position.z << std::endl;

    platePlane->modelMatrix = glm::translate(platePlane->modelMatrix, this->position);
    
    // but these are in model space not world
    // we want collision to be done in world space 
    platePlane->pointA += offset;
    platePlane->pointB += offset;
    platePlane->pointC += offset;
    platePlane->pointD += offset;

    for (const auto& p : this->constraintPoints) {
        p->setPosition(*p->getPosition() + offset);
        // change in position over change in time
        // should time be a global variable 
        glm::dvec3 vel = offset / timeStep;

        p->setVelocity(vel);
    }
}

void Plate::setPosition(glm::vec3 position) {
    double half = this->size * 0.5;

    platePlane->pointA = position + glm::vec3(-half, 0, -half); // top left
    platePlane->pointB = position + glm::vec3(half, 0, -half); // top right
    platePlane->pointC = position + glm::vec3(-half, 0, half); // bottom left
    platePlane->pointD = position + glm::vec3(half, 0, half); // bottom right
}


void Plate::setConstraintPoints(std::vector <MassPoint*> points) {
    this->constraintPoints = points;
}