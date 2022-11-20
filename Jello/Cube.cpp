
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

void Cube::reset() {
    setSpringMode(this->structuralSpring, this->shearSpring, this->bendSpring);
}

void Cube::addConnection(MassPoint **** massPointMap, MassPoint* point, int i, int j, int k) {
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
                // i inside so it fills points 
               /* back
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
                    1, 1, 1*/
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
                            // like placed on floor (fixed)
                            // TODO later add collision with plate so it can jiggle off the plate

                            point->setFixed(true);
                            bottomFace.push_back(point);
                        }
                    }
                    if (j == maxRes) {
                        // top
                        topFace.push_back(point);
                    }
                    if (k == maxRes) {
                        // front
                        frontFace.push_back(point);
                    }
                    if (k == 0) {
                        // back
                        backFace.push_back(point);
                    }
                }
                
                discretePoints.push_back(point);
                massPointMap[i][j][k] = point;
            }
        }
    }
    for (int i = 0; i < this->resolution; i++) {
        for (int j = 0; j < this->resolution; j++) {
            for (int k = 0; k < this->resolution; k++) {
                MassPoint* point = massPointMap[i][j][k];

                if (structural) {
                    /*Node(i, j, k) connected to
                        (i + 1, j, k), (i - 1, j, k), (i, j - 1, k), (i, j + 1, k), (i, j, k - 1), (i, j, k + 1)
                        (for surface nodes, some of these neighbors might not exists)*/

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

                    addConnection(massPointMap, point, i + 2, j, k);
                    addConnection(massPointMap, point, i, j + 2, k);
                    addConnection(massPointMap, point, i, j, k + 2);

                }

            }
        }
    }

    std::cout << "front" << std::endl;
    for (const auto& s : frontFace) {
        glm::dvec3* pos = s->getPosition();
        std::cout << pos->x << ", " << pos->y << ", " << pos->z << std::endl;
    }
    std::cout << "back" << std::endl;
    for (const auto& s : backFace) {
        glm::dvec3* pos = s->getPosition();
        std::cout << pos->x << ", " << pos->y << ", " << pos->z << std::endl;
    }
    std::cout << "right" << std::endl;
    for (const auto& s : rightFace) {
        glm::dvec3* pos = s->getPosition();
        std::cout << pos->x << ", " << pos->y << ", " << pos->z << std::endl;
    }
    std::cout << "left" << std::endl;
    for (const auto& s : leftFace) {
        glm::dvec3* pos = s->getPosition();
        std::cout << pos->x << ", " << pos->y << ", " << pos->z << std::endl;
    }
    std::cout << "top" << std::endl;
    for (const auto& s : topFace) {
        glm::dvec3* pos = s->getPosition();
        std::cout << pos->x << ", " << pos->y << ", " << pos->z << std::endl;
    }
    std::cout << "bottom" << std::endl;
    for (const auto& s : bottomFace) {
        glm::dvec3* pos = s->getPosition();
        std::cout << pos->x << ", " << pos->y << ", " << pos->z << std::endl;
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

// drawType : 0 = points, 1 = triangles 
void Cube::render(GLuint modelParameter, bool showDiscrete, int drawType) {

    std::vector <GLfloat> data{};
    std::vector <GLfloat> texData{};
    std::vector <GLfloat> normalData{};
    int dataSize;

    if (drawType == drawType::DRAWPOINT) {
        for (int i = 0; i < this->discretePoints.size(); i++) {
            MassPoint* massPoint = discretePoints[i];
            const glm::dvec3* pos = massPoint->getPosition();

            if (showDiscrete) {

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
        dataSize = data.size();
        // send data to GPU 
        glBufferData(GL_ARRAY_BUFFER, dataSize * sizeof(GLfloat), data.data(), GL_DYNAMIC_DRAW);
        //glUniformMatrix4fv(modelParameter, 1, GL_FALSE, glm::value_ptr(this->modelMatrix));

        // enable point size to have it accessible in shader
        //glEnable(GL_PROGRAM_POINT_SIZE);
        glPointSize(this->pointSize);
        glDrawArrays(GL_POINTS, 0, dataSize / 3); // TODO Really? can i divide 3 again? 
        //glDisable(GL_PROGRAM_POINT_SIZE);
    }
    else if (drawType == drawType::DRAWTRI) {
       
        // draw 2 triangles per square face
        //(const auto& face : faces)
        for (int i = 0; i < faces.size(); i++) {
            std::vector <MassPoint*>* face = faces[i];
            const int maxSize = face->size();
            for (int f = 0; f < face->size(); f++) {
                MassPoint* massPointA;
                MassPoint* massPointB;
                MassPoint* massPointC;
                glm::dvec3* posA; 
                glm::dvec3* posB;
                glm::dvec3* posC;
                glm::vec3 normal;
                // counter clockwise
                if ((f + 1) < maxSize && (f + resolution) < maxSize) {
                    // triangle 1
                    // point 1 
                    massPointA = (*face)[f];
                    posA = massPointA->getPosition();
                    data.push_back(posA->x);
                    data.push_back(posA->y);
                    data.push_back(posA->z);
                    // point 2
                    massPointB = (*face)[f + 1];
                    posB = massPointB->getPosition();
                    data.push_back(posB->x);
                    data.push_back(posB->y);
                    data.push_back(posB->z);
                    // point 3
                    massPointC = (*face)[f + resolution];
                    posC = massPointC->getPosition();
                    data.push_back(posC->x);
                    data.push_back(posC->y);
                    data.push_back(posC->z);

                    // texture 
                    texData.push_back(0); // u
                    texData.push_back(0); // v
                    texData.push_back(0); // u
                    texData.push_back(0); // v
                    texData.push_back(0); // u
                    texData.push_back(0); // v

                    // normal
                    normal = glm::cross(*posB - *posA, *posC - *posA); // point 2 - point 1  x  point 3 - point 1
                    normal = glm::normalize(normal);
                    normalData.push_back(normal.x); // x
                    normalData.push_back(normal.y); // y
                    normalData.push_back(normal.z); // z
                    normalData.push_back(normal.x); // x
                    normalData.push_back(normal.y); // y
                    normalData.push_back(normal.z); // z 
                    normalData.push_back(normal.x); // x
                    normalData.push_back(normal.y); // y
                    normalData.push_back(normal.z); // z
                }

                if ((f + 1) < maxSize && (f + resolution) < maxSize && (f + resolution + 1) < maxSize) {
                    // triangle 2
                    // point 4
                    massPointA = (*face)[f + 1];
                    posA = massPointA->getPosition();
                    data.push_back(posA->x);
                    data.push_back(posA->y);
                    data.push_back(posA->z);

                    // point 5
                    massPointB = (*face)[f + resolution + 1];
                    posB = massPointB->getPosition();
                    data.push_back(posB->x);
                    data.push_back(posB->y);
                    data.push_back(posB->z);

                    // point 6
                    massPointC = (*face)[f + resolution];
                    posC = massPointC->getPosition();
                    data.push_back(posC->x);
                    data.push_back(posC->y);
                    data.push_back(posC->z);

                    // texture
                    texData.push_back(i + f); // u
                    texData.push_back(i + f + 1); // v
                    texData.push_back(i + f); // u
                    texData.push_back(i + f + 1); // v
                    texData.push_back(i + f); // u
                    texData.push_back(i + f + 1); // v

                    // normal (TODO: put in func?)
                    normal = glm::cross(*posB - *posA, *posC - *posA); // point 2 - point 1  x  point 3 - point 1
                    normal = glm::normalize(normal);
                    normalData.push_back(normal.x); // x
                    normalData.push_back(normal.y); // y
                    normalData.push_back(normal.z); // z
                    normalData.push_back(normal.x); // x
                    normalData.push_back(normal.y); // y
                    normalData.push_back(normal.z); // z 
                    normalData.push_back(normal.x); // x
                    normalData.push_back(normal.y); // y
                    normalData.push_back(normal.z); // z
                }
            }
        }
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glEnableVertexAttribArray(posLoc);
        dataSize = data.size();
        // send data to GPU 
        glBufferData(GL_ARRAY_BUFFER, dataSize * sizeof(GLfloat), data.data(), GL_STATIC_DRAW);
        //glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, texVBO);
       glEnableVertexAttribArray(texCoordLoc);
        glBufferData(GL_ARRAY_BUFFER, texData.size() * sizeof(GLfloat), texData.data(), GL_STATIC_DRAW);
        //glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, 0, 0, 0);
        
        glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
        glEnableVertexAttribArray(normalLoc);
        glBufferData(GL_ARRAY_BUFFER, normalData.size() * sizeof(GLfloat), normalData.data(), GL_STATIC_DRAW);
        //glVertexAttribPointer(normalLoc, 3, GL_FLOAT, 0, 0, 0);

        glDrawArrays(GL_TRIANGLES, 0, dataSize / 3); // TODO Really? can i divide 3 again? 
    }

    data.clear();
    texData.clear();
    normalData.clear();

    if (showSpring) {
        // show springs
        for (int i = 0; i < this->discretePoints.size(); i++) {
            MassPoint* massPoint = discretePoints[i];
            const glm::dvec3* pos = massPoint->getPosition();

            if (showDiscrete) {

                const int cc = massPoint->getConnectionCount();
                for (int c = 0; c < cc; c++) {
                    MassPoint* connection = massPoint->getConnection(c);

                    data.push_back(pos->x);
                    data.push_back(pos->y);
                    data.push_back(pos->z);

                    const glm::dvec3* cpos = connection->getPosition();

                    data.push_back(cpos->x);
                    data.push_back(cpos->y);
                    data.push_back(cpos->z);
                }

            }
            else {
                // only show surface
                if (massPoint->isSurfacePoint()) {

                    const int cc = massPoint->getConnectionCount();
                    for (int c = 0; c < cc; c++) {
                        MassPoint* connection = massPoint->getConnection(c);

                        if (connection->isSurfacePoint()) {

                            data.push_back(pos->x);
                            data.push_back(pos->y);
                            data.push_back(pos->z);

                            const glm::dvec3* cpos = connection->getPosition();

                            data.push_back(cpos->x);
                            data.push_back(cpos->y);
                            data.push_back(cpos->z);
                        }
                    }
                }
            }
            dataSize = int(data.size());
            glBufferData(GL_ARRAY_BUFFER, dataSize * sizeof(GLfloat), data.data(), GL_DYNAMIC_DRAW);
        }
        //glUniformMatrix4fv(modelParameter, 1, GL_FALSE, glm::value_ptr(this->modelMatrix));

        data.clear();
        // enable point size to have it accessible in shader
        //glEnable(GL_PROGRAM_POINT_SIZE);
        glPointSize(this->pointSize);
        glDrawArrays(GL_LINES, 0, dataSize / 3); // TODO Really? can i divide 3 again? 
    }
}

void Cube::initArrays() {
    // init buffers
    GLint program = -1;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);

    // attribute locations
    glBindAttribLocation(program, posLoc, "pos_attrib");
    glBindAttribLocation(program, texCoordLoc, "tex_coord_attrib");
    glBindAttribLocation(program, normalLoc, "normal_attrib");

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

