
#include "Cube.h"

#include <iostream>

Cube::Cube(int resolution, glm::vec3 position, GLint shader, GLint debug) {
    this->resolution = resolution;
    this->position = position;
    this->modelMatrix = glm::translate(glm::mat4(1.0f), this->position);
    // initial position only because mass points moves in cpu 

    this->shaderProgram = shader;
    this->debugShaderProgram = debug;

    initArrays();
}

Cube::Cube() {
    this->modelMatrix = glm::translate(glm::mat4(1.0f), this->position);

    initArrays();
}

void Cube::reset() {
    setSpringMode(this->structuralSpring, this->shearSpring, this->bendSpring);
}

void Cube::addConnection(MassPoint **** massPointMap, MassPoint* point, int i, int j, int k) {
    // for surface nodes, some of these neighbors might not exists
    if (i < resolution && j < resolution && k < resolution && i >= 0 && j >= 0 && k >= 0) {
        point->addConnection(massPointMap[i][j][k]);
    }
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

    // create a storage space to store the points
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
    for (int j = 0; j < this->resolution; j++) {
        for (int k = 0; k < this->resolution; k++) {
            for (int i = 0; i < this->resolution; i++) {
                // i inside so it fills points along the x axis first, then z then y 
                /* EXAMPLE:
                    back
                        0, 0, 0
                        0.5, 0, 0
                        1, 0, 0
                        0, 0.5, 0
                        0.5, 0.5, 0
                        1, 0.5, 0
                        0, 1, 0
                        0.5, 1, 0
                        1, 1, 0
                    right
                        1, 0, 0
                        1, 0, 0.5
                        1, 0, 1
                        1, 0.5, 0
                        1, 0.5, 0.5
                        1, 0.5, 1
                        1, 1, 0
                        1, 1, 0.5
                        1, 1, 1
                */
                bool isSurface = i * j * k * (maxRes - i) * (maxRes - j) * (maxRes - k) == 0;
                
                // store pointers
                MassPoint* point = new MassPoint(glm::vec3(float(i) / float(maxRes), float(j) / float(maxRes), float(k) / float(maxRes)), isSurface);
                
                // get sides
                if (isSurface) {

                    // corner/edge points can be in two faces
                    if (i == 0) {
                        // left
                        leftFace.push_back(point);
                    }
                    if (i == maxRes) {
                        // right
                        rightFace.push_back(point);
                    }
                    if (j == 0) {
                        // bottom
                        if (this->fixedFloor) {
                            point->setFixed(true);
                        }
                        bottomFace.push_back(point);
                    }
                    if (j == maxRes) {
                        // top
                        topFace.push_back(point);
                    }
                    if (k == 0) {
                        // back
                        backFace.push_back(point);
                    }
                    if (k == maxRes) {
                        // front
                        frontFace.push_back(point);
                    }
                }
                
                discretePoints.push_back(point);
                massPointMap[i][j][k] = point;
            }
        }
    }

    // to print out points in each face
    /*std::cout << "front: " << frontFace.size() << std::endl;
    for (int i = 0; i < frontFace.size(); i++) {
        MassPoint* point = frontFace[i];
        const glm::dvec3* pos = point->getPosition();
        std::cout << pos->x << ", " << pos->y << ", " << pos->z << std::endl;
    }
    std::cout << "--------" << std::endl;
    std::cout << "back: " << backFace.size() << std::endl;
    for (int i = 0; i < backFace.size(); i++) {
        MassPoint* point = backFace[i];
        const glm::dvec3* pos = point->getPosition();
        std::cout << pos->x << ", " << pos->y << ", " << pos->z << std::endl;
    }
    std::cout << "--------" << std::endl;
    std::cout << "right: " <<rightFace.size() << std::endl;
    for (int i = 0; i < rightFace.size(); i++) {
        MassPoint* point = rightFace[i];
        const glm::dvec3* pos = point->getPosition();
        std::cout << pos->x << ", " << pos->y << ", " << pos->z << std::endl;
    }
    std::cout << "--------" << std::endl;
    std::cout << "left: " << leftFace.size() << std::endl;
    for (int i = 0; i < leftFace.size(); i++) {
        MassPoint* point = leftFace[i];
        const glm::dvec3* pos = point->getPosition();
        std::cout << pos->x << ", " << pos->y << ", " << pos->z << std::endl;
    }
    std::cout << "--------" << std::endl;
    std::cout << "top: " << topFace.size() << std::endl;
    for (int i = 0; i < topFace.size(); i++) {
        MassPoint* point = topFace[i];
        const glm::dvec3* pos = point->getPosition();
        std::cout << pos->x << ", " << pos->y << ", " << pos->z << std::endl;
    }
    std::cout << "--------" << std::endl;
    std::cout << "bottom: " << bottomFace.size() << std::endl;
    for (int i = 0; i < bottomFace.size(); i++) {
        MassPoint* point = bottomFace[i];
        const glm::dvec3* pos = point->getPosition();
        std::cout << pos->x << ", " << pos->y << ", " << pos->z << std::endl;
    }
    std::cout << "--------" << std::endl;*/

    // add spring connections
    for (int i = 0; i < this->resolution; i++) {
        for (int j = 0; j < this->resolution; j++) {
            for (int k = 0; k < this->resolution; k++) {
                MassPoint* point = massPointMap[i][j][k];

                if (structural) {
                    /* Node(i, j, k) connected to
                        (i + 1, j, k), (i - 1, j, k), (i, j - 1, k), 
                        (i, j + 1, k), (i, j, k - 1), (i, j, k + 1)
                    */
                    addConnection(massPointMap, point, i + 1, j, k);
                    addConnection(massPointMap, point, i, j + 1, k);
                    addConnection(massPointMap, point, i, j, k + 1);
                }

                if (shear) {
                    // Every node connected to its diagonal neighbors

                    addConnection(massPointMap, point, i + 1, j + 1, k);
                    addConnection(massPointMap, point, i - 1, j + 1, k);
                    addConnection(massPointMap, point, i, j + 1, k + 1);

                    addConnection(massPointMap, point, i, j - 1, k + 1);
                    addConnection(massPointMap, point, i + 1, j, k + 1);
                    addConnection(massPointMap, point, i - 1, j, k + 1);

                    addConnection(massPointMap, point, i + 1, j + 1, k + 1);
                    addConnection(massPointMap, point, i - 1, j + 1, k + 1);
                    addConnection(massPointMap, point, i - 1, j - 1, k + 1);
                    addConnection(massPointMap, point, i + 1, j - 1, k + 1);
                }

                if (bend) {
                    // Every node connected to its second neighbor in every direction
                    // (6 connections per node, unless surface node)
                    // only adding the positive ones to itself

                    addConnection(massPointMap, point, i + 2, j, k);
                    addConnection(massPointMap, point, i, j + 2, k);
                    addConnection(massPointMap, point, i, j, k + 2);
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

void Cube::setExternalForce(glm::dvec3 force) {
    for (int i = 0; i < discretePoints.size(); i++) {
        MassPoint* currentPoint = discretePoints[i];

        if (currentPoint->isFixed() == true) {
            // TODO: do we need this check? 
            continue;
        }

        currentPoint->setExternalForce(force); // at position? -> should get point of collision
    }
}

void Cube::addTriangle(glm::dvec3* posA, glm::dvec3* posB, glm::dvec3* posC) {

    // normal
    glm::dvec3 normal = glm::cross(*posB - *posA, *posC - *posA); // point 2 - point 1  x  point 3 - point 1
    normal = glm::normalize(normal);

    // point 1
    // position
    this->data.push_back(posA->x);
    this->data.push_back(posA->y);
    this->data.push_back(posA->z);
    // uv texture coord
    this->texData.push_back(0); // u
    this->texData.push_back(0); // v
    // normal
    this->normalData.push_back(normal.x); // x
    this->normalData.push_back(normal.y); // y
    this->normalData.push_back(normal.z); // z
    
    // point 2
    // position
    this->data.push_back(posB->x);
    this->data.push_back(posB->y);
    this->data.push_back(posB->z);
    // uv texture coord
    this->texData.push_back(0); // u
    this->texData.push_back(0); // v
    // normal
    this->normalData.push_back(normal.x); // x
    this->normalData.push_back(normal.y); // y
    this->normalData.push_back(normal.z); // z

    // point 3
    // position
    this->data.push_back(posC->x);
    this->data.push_back(posC->y);
    this->data.push_back(posC->z);
    // uv texture coord
    this->texData.push_back(0); // u
    this->texData.push_back(0); // v
    // normal
    this->normalData.push_back(normal.x); // x
    this->normalData.push_back(normal.y); // y
    this->normalData.push_back(normal.z); // z

    /*std::cout << posA->x << ", " << posA->y << ", " << posA->z << std::endl;
    std::cout << posB->x << ", " << posB->y << ", " << posB->z << std::endl;
    std::cout << posC->x << ", " << posC->y << ", " << posC->z << std::endl;
    std::cout << "Normal :" << normal.x << ", " << normal.y << ", " << normal.z << std::endl;
    std::cout << "-------" << std::endl;*/
}

// drawType : 0 = points, 1 = triangles 
void Cube::render(GLuint modelParameter, bool showDiscrete, bool showSpring, bool debugMode) {

    glUniformMatrix4fv(modelParameter, 1, false, glm::value_ptr(this->modelMatrix));

    glBindVertexArray(VAO);

    if (debugMode) {
        // only draw points, including showing discrete points 

        for (int i = 0; i < this->discretePoints.size(); i++) {
            MassPoint* massPoint = discretePoints[i];
            const glm::dvec3* pos = massPoint->getPosition();

            if (showDiscrete) {
                // show mass points inside the surface
                data.push_back(pos->x);
                data.push_back(pos->y);
                data.push_back(pos->z);
            }
            else {
                // only show surface
                if (massPoint->isSurfacePoint()) {

                    data.push_back(pos->x);
                    data.push_back(pos->y);
                    data.push_back(pos->z);
                }
            }
        }

        // send data to GPU 
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glEnableVertexAttribArray(debugPosLoc);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), data.data(), GL_DYNAMIC_DRAW);
        // draw points
        glPointSize(this->pointSize);
        glDrawArrays(GL_POINTS, 0, this->data.size() / 3);

        this->data.clear();

        if (showSpring) {
            // show springs
            for (int i = 0; i < this->discretePoints.size(); i++) {
                MassPoint* massPoint = discretePoints[i];
                const glm::dvec3* pos = massPoint->getPosition();
                const int cc = massPoint->getConnectionCount();

                if (showDiscrete) {
                    for (int c = 0; c < cc; c++) {
                        MassPoint* connection = massPoint->getConnection(c);

                        this->data.push_back(pos->x);
                        this->data.push_back(pos->y);
                        this->data.push_back(pos->z);

                        const glm::dvec3* cpos = connection->getPosition();

                        this->data.push_back(cpos->x);
                        this->data.push_back(cpos->y);
                        this->data.push_back(cpos->z);
                    }
                }
                else {
                    // only show surface connection with surface
                    if (massPoint->isSurfacePoint()) {
                        for (int c = 0; c < cc; c++) {
                            MassPoint* connection = massPoint->getConnection(c);

                            if (connection->isSurfacePoint()) {

                                this->data.push_back(pos->x);
                                this->data.push_back(pos->y);
                                this->data.push_back(pos->z);

                                const glm::dvec3* cpos = connection->getPosition();

                                this->data.push_back(cpos->x);
                                this->data.push_back(cpos->y);
                                this->data.push_back(cpos->z);
                            }
                        }
                    }
                }
            }

            glBufferData(GL_ARRAY_BUFFER, this->data.size() * sizeof(GLfloat), this->data.data(), GL_DYNAMIC_DRAW);
            glPointSize(this->pointSize);
            glDrawArrays(GL_LINES, 0, this->data.size() / 3);

            this->data.clear();
        }
    }
    else {
        // draw only surface (triangle faces)

        for (int i = 0; i < frontFaces.size(); i++) {
            std::vector <MassPoint*>* face = frontFaces[i];
            const int maxSize = face->size();
            for (int f = 0; f < maxSize; f++) {
                MassPoint* massPointA;
                MassPoint* massPointB;
                MassPoint* massPointC;
                glm::dvec3* posA;
                glm::dvec3* posB;
                glm::dvec3* posC;

                // draw 2 triangles per square face
                // counter clockwise winding order

                // if not edge 
                // + 1 cos we start from 0 
                if (((f+1) % this->resolution) != 0) {
                    // if there is a row of points above, since we start drawing from bottom left point 
                    if ((f + resolution) < maxSize) {
                        // triangle 1
                        // point 1 
                        massPointA = (*face)[f];
                        posA = massPointA->getPosition();
                        massPointB = (*face)[f + 1];
                        posB = massPointB->getPosition();
                        massPointC = (*face)[f + resolution];
                        posC = massPointC->getPosition();
                        addTriangle(posA, posB, posC);

                        // triangle 2
                        massPointA = (*face)[f+1];
                        posA = massPointA->getPosition();
                        massPointB = (*face)[f + 1 + resolution];
                        posB = massPointB->getPosition();
                        massPointC = (*face)[f + resolution];
                        posC = massPointC->getPosition();
                        addTriangle(posA, posB, posC);
                    }
                }
            }
        }

        for (int i = 0; i < backFaces.size(); i++) {
            std::vector <MassPoint*>* face = backFaces[i];
            const int maxSize = face->size();
            for (int f = 0; f < maxSize; f++) {
                MassPoint* massPointA;
                MassPoint* massPointB;
                MassPoint* massPointC;
                glm::dvec3* posA;
                glm::dvec3* posB;
                glm::dvec3* posC;
                // counter clockwise

                if (((f + 1) % this->resolution) != 0) {

                    if ((f + resolution) < maxSize) {
                        // triangle 1
                        massPointA = (*face)[f + resolution];
                        posA = massPointA->getPosition();
                        massPointB = (*face)[f + 1];
                        posB = massPointB->getPosition();
                        massPointC = (*face)[f];
                        posC = massPointC->getPosition();
                        addTriangle(posA, posB, posC);

                        // triangle 2
                        massPointA = (*face)[f + resolution];
                        posA = massPointA->getPosition();
                        massPointB = (*face)[f + 1 + resolution];
                        posB = massPointB->getPosition();
                        massPointC = (*face)[f + 1];
                        posC = massPointC->getPosition();
                        addTriangle(posA, posB, posC);
                    }
                }

            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glEnableVertexAttribArray(posLoc);
        // send data to GPU 
        glBufferData(GL_ARRAY_BUFFER, this->data.size() * sizeof(GLfloat), this->data.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, texVBO);
        glEnableVertexAttribArray(texCoordLoc);
        glBufferData(GL_ARRAY_BUFFER, this->texData.size() * sizeof(GLfloat), this->texData.data(), GL_DYNAMIC_DRAW);
        
        glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
        glEnableVertexAttribArray(normalLoc);
        glBufferData(GL_ARRAY_BUFFER, this->normalData.size() * sizeof(GLfloat), this->normalData.data(), GL_DYNAMIC_DRAW);

        glDrawArrays(GL_TRIANGLES, 0, this->data.size() / 3); // drawing dataSize / 3 triangles (3 points form a triangle)

        this->data.clear();
        this->texData.clear();
        this->normalData.clear();
    }

    // unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Cube::initArrays() {
    // init buffers

    // attribute locations
    glBindAttribLocation(shaderProgram, posLoc, "pos_attrib");
    glBindAttribLocation(shaderProgram, texCoordLoc, "tex_coord_attrib");
    glBindAttribLocation(shaderProgram, normalLoc, "normal_attrib");

    // VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // vertex position 
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //dataSize = int(this->discretePoints.size());
    // send to GPU
    //glBufferData(GL_ARRAY_BUFFER, dataSize * sizeof(GLfloat), this->discretePoints.data(), GL_DYNAMIC_DRAW);
    //data.clear();

    // texture 
    glGenBuffers(1, &texVBO);
    glBindBuffer(GL_ARRAY_BUFFER, texVBO);
    glEnableVertexAttribArray(texCoordLoc);
    glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, 0, 0, 0);

    // normal 
    glGenBuffers(1, &normalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glEnableVertexAttribArray(normalLoc);
    glVertexAttribPointer(normalLoc, 3, GL_FLOAT, 0, 0, 0);

    // debug shader 
    glBindAttribLocation(debugShaderProgram, debugPosLoc, "pos_attrib");
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(debugPosLoc);
    glVertexAttribPointer(debugPosLoc, 3, GL_FLOAT, 0, 0, 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Cube::setSpringMode(bool structural, bool shear, bool bend) {
    this->discretePoints.clear();
    for (const auto& f : frontFaces) {
        f->clear();
    }
    for (const auto& f : backFaces) {
        f->clear();
    }
    this->fillDiscretePoints(structural, shear, bend);
    this->structuralSpring = structural;
    this->shearSpring = shear;
    this->bendSpring = bend;
}

