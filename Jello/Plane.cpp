#include "Plane.h"
#include<iostream>

Plane::Plane(glm::vec3 pointA, glm::vec3 pointB, glm::vec3 pointC, glm::vec3 pointD, GLuint debugShader) {
    // finite plane

    this->pointA = pointA; // top left
    this->pointB = pointB; // top right
    this->pointC = pointC; // bottom left
    this->pointD = pointD; // bottom right
    this->pointInPlane = pointA;

    this->normal = glm::normalize(glm::cross(pointD - pointC, pointA - pointC));
    this->debugShader = debugShader;

    this->initArrays();
}

void Plane::render(GLuint modelParameter) {

    glUniformMatrix4fv(modelParameter, 1, false, glm::value_ptr(this->modelMatrix));

    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glEnableVertexAttribArray(debugPosLoc);

    // draw as lines (show only frame)
    glDrawArrays(GL_LINES, 0, this->dataSize / 3);

    // unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
};

void Plane::setPosition(glm::vec3 pos) {
    this->position = pos;
    this->modelMatrix = glm::translate(glm::mat4(1.0), this->position);
}

glm::vec3 Plane::getPosition() {
    return this->position;
}

void Plane::initArrays() {
    // init buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // attribute locations
    glBindAttribLocation(debugShader, debugPosLoc, "pos_attrib");
    glEnableVertexAttribArray(debugPosLoc);
    glVertexAttribPointer(debugPosLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // send to GPU
    // rendering as 4 lines
    // line 1
    this->data.push_back(this->pointA.x);
    this->data.push_back(this->pointA.y);
    this->data.push_back(this->pointA.z);

    this->data.push_back(this->pointC.x);
    this->data.push_back(this->pointC.y);
    this->data.push_back(this->pointC.z);

    // line 2
    this->data.push_back(this->pointB.x);
    this->data.push_back(this->pointB.y);
    this->data.push_back(this->pointB.z);

    this->data.push_back(this->pointD.x);
    this->data.push_back(this->pointD.y);
    this->data.push_back(this->pointD.z);

    // line 3
    this->data.push_back(this->pointC.x);
    this->data.push_back(this->pointC.y);
    this->data.push_back(this->pointC.z);

    this->data.push_back(this->pointD.x);
    this->data.push_back(this->pointD.y);
    this->data.push_back(this->pointD.z);

    // line 4
    this->data.push_back(this->pointA.x);
    this->data.push_back(this->pointA.y);
    this->data.push_back(this->pointA.z);

    this->data.push_back(this->pointB.x);
    this->data.push_back(this->pointB.y);
    this->data.push_back(this->pointB.z);

    this->dataSize = this->data.size();

    // send data to GPU
    glBufferData(GL_ARRAY_BUFFER, this->dataSize * sizeof(GLfloat), this->data.data(), GL_STATIC_DRAW);

    this->data.clear();

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}