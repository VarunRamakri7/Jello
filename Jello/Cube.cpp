
#include "Cube.h"

#include <iostream>

Cube::Cube(int resolution) {
    this->resolution = resolution;
    this->modelMatrix = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(this->scale)), this->position);

    initArrays();
}

Cube::Cube() {
    this->modelMatrix = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(this->scale)), this->position);

    initArrays();
}

glm::mat4 Cube::getModelMatrix() {
    return this->modelMatrix;
}

void Cube::setFixedFloor(bool set) {
    this->fixedFloor = set;
    setSpringMode(this->structuralSpring, this->structuralSpring, this->structuralSpring);
}

bool Cube::getFixedFloor() {
    return this->fixedFloor;
}

void Cube::fillDiscretePoints(bool structural, bool shear, bool bend) {
    const int maxRes = this->resolution > 1 ? this->resolution - 1 : 1;
    // center is not center
    if (this->resolution == 1) {
        /*discretePoints = {
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f)
        };
        surfacePoints = {
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f)
        };*/
    }

    MassPoint**** massPointMap = new MassPoint * **[this->resolution];
    // make 3d array to store relationship massPoints
    for (int i = 0; i < this->resolution; ++i) {
        massPointMap[i] = new MassPoint * *[this->resolution];
        for (int j = 0; j < this->resolution; ++j) {
            massPointMap[i][j] = new MassPoint * [this->resolution];
            for (int k = 0; k < this->resolution; ++k) {
                massPointMap[i][j][k] = new MassPoint[this->resolution];
            }
        }
    }

    // fill points
    for (int i = 0; i < this->resolution; i++) {
        for (int j = 0; j < this->resolution; j++) {
            for (int k = 0; k < this->resolution; k++) {
                bool isSurface = i * j * k * (maxRes - i) * (maxRes - j) * (maxRes - k) == 0;
                // store pointers
                MassPoint* point = new MassPoint(glm::vec3(float(i) / float(maxRes), float(j) / float(maxRes), float(k) / float(maxRes)), isSurface);
                if (this->fixedFloor && j == 0) {
                    // like placed on floor (fixed)
                    // TODO later add collision with plate so it can jiggle off the plate
                    point->setFixed(true);
                }
                discretePoints.push_back(point);
                massPointMap[i][j][k] = point;

            }
        }
    }

    // add connections
    // these are diff connections anyways, so dont need to check for redundancy
    if (structural) {
        /*Node(i, j, k) connected to
            (i + 1, j, k), (i - 1, j, k), (i, j - 1, k), (i, j + 1, k), (i, j, k - 1), (i, j, k + 1)
            (for surface nodes, some of these neighbors might not exists)*/

        for (int i = 0; i < this->resolution; i++) {
            for (int j = 0; j < this->resolution; j++) {
                for (int k = 0; k < this->resolution; k++) {
                    MassPoint* point = massPointMap[i][j][k];
                    if ((i + 1) < resolution) {
                        point->addConnection(massPointMap[i + 1][j][k]);
                    }
                    if ((i - 1) >= 0) {
                        point->addConnection(massPointMap[i - 1][j][k]);
                    }
                    if ((j - 1) >= 0) {
                        point->addConnection(massPointMap[i][j - 1][k]);
                    }
                    if ((j + 1) < resolution) {
                        point->addConnection(massPointMap[i][j + 1][k]);
                    }
                    if ((k - 1) >= 0) {
                        point->addConnection(massPointMap[i][j][k - 1]);
                    }
                    if ((k + 1) < resolution) {
                        point->addConnection(massPointMap[i][j][k + 1]);
                    }

                }
            }
        }
    }
    if (shear) {
        // Every node connected to its diagonal neighbors
        for (int i = 0; i < this->resolution; i++) {
            for (int j = 0; j < this->resolution; j++) {
                for (int k = 0; k < this->resolution; k++) {
                    MassPoint* point = massPointMap[i][j][k];
                    if ((i + 1) < this->resolution && (j + 1) < this->resolution) {
                        point->addConnection(massPointMap[i + 1][j + 1][k]);
                    }
                    if ((i + 1) < this->resolution && (k + 1) < this->resolution) {
                        point->addConnection(massPointMap[i + 1][j][k + 1]);
                    }
                    if ((j + 1) < this->resolution && (k + 1) < this->resolution) {
                        point->addConnection(massPointMap[i][j + 1][k + 1]);
                    }
                    if ((i - 1) >= 0 && (j - 1) >= 0) {
                        point->addConnection(massPointMap[i - 1][j - 1][k]);
                    }
                    if ((k - 1) >= 0 && (j - 1) >= 0) {
                        point->addConnection(massPointMap[i][j - 1][k - 1]);
                    }
                    if ((i - 1) >= 0 && (k - 1) >= 0) {
                        point->addConnection(massPointMap[i - 1][j][k - 1]);
                    }

                }
            }
        }
    }
    if (bend) {
        // Every node connected to its second neighbor in every direction
        // (6 connections per node, unless surface node)
        for (int i = 0; i < this->resolution; i++) {
            for (int j = 0; j < this->resolution; j++) {
                for (int k = 0; k < this->resolution; k++) {
                    MassPoint* point = massPointMap[i][j][k];
                    if ((i + 2) < resolution) {
                        point->addConnection(massPointMap[i + 2][j][k]);
                    }
                    if ((i - 2) >= 0) {
                        point->addConnection(massPointMap[i - 2][j][k]);
                    }
                    if ((j - 2) >= 0) {
                        point->addConnection(massPointMap[i][j - 2][k]);
                    }
                    if ((j + 2) < resolution) {
                        point->addConnection(massPointMap[i][j + 2][k]);
                    }
                    if ((k - 2) >= 0) {
                        point->addConnection(massPointMap[i][j][k - 2]);
                    }
                    if ((k + 2) < resolution) {
                        point->addConnection(massPointMap[i][j][k + 2]);
                    }
                }
            }
        }
    }
}


void Cube::resetAcceleration() {
    // reset acceleration for all points
    for (int i = 0; i < discretePoints.size(); i++) {
        MassPoint* currentPoint = discretePoints[i];
        currentPoint->setAcceleration(glm::dvec3(0));
    }
}

void Cube::setExternalForce(glm::vec3 force) {
    for (int i = 0; i < discretePoints.size(); i++) {
        MassPoint* currentPoint = discretePoints[i];

        if (currentPoint->isFixed() == true) {
            // TODO: do we need this check? 
            continue;
        }

        currentPoint->setExternalForce(force); // at position? -> should get point of collision
    }
}

void Cube::render(GLuint modelParameter, bool showDiscrete) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);

    for (int i = 0; i < this->discretePoints.size(); i++) {
        MassPoint* massPoint = discretePoints[i];
        const glm::dvec3* pos = massPoint->getPosition();

        if (showDiscrete) {

            this->data.push_back(pos->x);
            this->data.push_back(pos->y);
            this->data.push_back(pos->z);

        }
        else {
            // only show surface
            if (massPoint->isSurfacePoint()) {

                this->data.push_back(pos->x);
                this->data.push_back(pos->y);
                this->data.push_back(pos->z);
            }
        }
        this->dataSize = int(this->data.size());
        glBufferData(GL_ARRAY_BUFFER, this->dataSize * sizeof(GLfloat), this->data.data(), GL_DYNAMIC_DRAW);
    }
    //glUniformMatrix4fv(modelParameter, 1, GL_FALSE, glm::value_ptr(this->modelMatrix));

    this->data.clear();
    // enable point size
    // 
    //glEnable(GL_PROGRAM_POINT_SIZE);
    glPointSize(5);
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

void Cube::setSpringMode(bool structural, bool shear, bool bend) {
    this->discretePoints.clear();
    this->fillDiscretePoints(structural, shear, bend);
    this->structuralSpring = structural;
    this->shearSpring = shear;
    this->bendSpring = bend;
}

