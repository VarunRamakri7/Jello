
#include "Cube.h"

#include <iostream>

Cube::Cube(int resolution) {
    this->resolution = resolution;
    this->modelMatrix = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(this->scale)), this->position);
    
    fillDiscretePoints();
    
    initArrays();
}

glm::mat4 Cube::getModelMatrix() {
    return this->modelMatrix;
}

void Cube::fillDiscretePoints() {
    int maxRes = resolution > 1 ? 1 : resolution - 1;
    // center is not center
    if (resolution == 1) {
        /*discretePoints = {
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f)
        };
        surfacePoints = {
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f)
        };*/
    }
    
    for (int i=0; i<this->resolution; i++) {
        for (int j=0; j<this->resolution; j++) {
            for (int k=0; k<this->resolution; k++) {
                //discretePoints.push_back(glm::vec3(i/maxRes, j/maxRes, k/maxRes));
                discretePoints.push_back(i / maxRes);
                discretePoints.push_back(j / maxRes);
                discretePoints.push_back(k / maxRes);
                if (i * j * k * (7 - i) * (7 - j) * (7 - k) == 0) {
                    // surface points 
                    //surfacePoints.push_back(glm::vec3(i / maxRes, j / maxRes, k / maxRes));
                    surfacePoints.push_back(i / maxRes);
                    surfacePoints.push_back(j / maxRes);
                    surfacePoints.push_back(k / maxRes);
                }
            }
        }
    }
}

void Cube::getSurface() {

    for (int i = 0; i < this->resolution; i++) {
        for (int j = 0; j < this->resolution; j++) {
            for (int k = 0; k < this->resolution; k++) {
                
            }
        }
    }
}

void Cube::render(GLuint modelParameter, bool showSurface) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    //glUniformMatrix4fv(modelParameter, 1, GL_FALSE, glm::value_ptr(this->modelMatrix));
    if (showSurface) {
        this->dataSize = int(this->surfacePoints.size());
        glBufferData(GL_ARRAY_BUFFER, this->dataSize * sizeof(GLfloat), this->surfacePoints.data(), GL_DYNAMIC_DRAW);
    }
    else {
        this->dataSize = int(this->discretePoints.size());
        glBufferData(GL_ARRAY_BUFFER, this->dataSize * sizeof(GLfloat), this->discretePoints.data(), GL_DYNAMIC_DRAW);
    }
    
    // enable point size
    // 
    //glEnable(GL_PROGRAM_POINT_SIZE);
    glPointSize(10);
    glDrawArrays(GL_POINTS, 0, this->dataSize / 3); // TODO Really? can i divide 3 again? 
    //glDisable(GL_PROGRAM_POINT_SIZE);
}

void Cube::initArrays() {
    // init buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    //this->dataSize = int(this->discretePoints.size());
    // send to GPU
    //glBufferData(GL_ARRAY_BUFFER, this->dataSize * sizeof(GLfloat), this->discretePoints.data(), GL_DYNAMIC_DRAW);
    //this->data.clear();

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

glm::vec4 Cube::getModelCoord() {
    return this->modelMatrix * glm::vec4(0, 0, 0, 1.0);
}