#include "Plane.h"

Plane::Plane(glm::vec3 pointA, glm::vec3 pointB, glm::vec3 pointC, glm::vec3 pointD) {
    // finite plane

    this->pointA = pointA; // top left
    this->pointB = pointB; // top right
    this->pointC = pointC; // bottom left
    this->pointD = pointD; // bottom right
    this->pointInPlane = pointA;

    this->normal = glm::normalize(glm::cross(pointD - pointC, pointA - pointC));

    this->initArrays();
}

void Plane::render(GLuint modelParameter) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
  
    // enable point size
    // 
    //glEnable(GL_PROGRAM_POINT_SIZE);
    glPointSize(5);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, this->dataSize / 3); // TODO Really? can i divide 3 again? 
    //glDisable(GL_PROGRAM_POINT_SIZE)
};

glm::mat4 Plane::getModelMatrix() {
    return this->modelMatrix;
}

void Plane::initArrays() {
    // init buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // send to GPU
    // rendering as triangle strip
    this->data.push_back(this->pointA.x);
    this->data.push_back(this->pointA.y);
    this->data.push_back(this->pointA.z);

    this->data.push_back(this->pointC.x);
    this->data.push_back(this->pointC.y);
    this->data.push_back(this->pointC.z);

    this->data.push_back(this->pointB.x);
    this->data.push_back(this->pointB.y);
    this->data.push_back(this->pointB.z);

    this->data.push_back(this->pointD.x);
    this->data.push_back(this->pointD.y);
    this->data.push_back(this->pointD.z);

    this->dataSize = this->data.size();

    glBufferData(GL_ARRAY_BUFFER, this->dataSize * sizeof(GLfloat), this->data.data(), GL_DYNAMIC_DRAW);

    this->data.clear();

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}