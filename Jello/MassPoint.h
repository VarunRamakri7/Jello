#ifndef MASSPOINT_H
#define MASSPOINT_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

class MassPoint {
public:
    MassPoint(glm::vec3 position, bool isSurfacePoint);
    MassPoint(){}; // default constructor 
    ~MassPoint(){};

    // Set
    void setPosition(glm::vec3 position);
    void setVelocity(glm::vec3 velocity);
    void setAcceleration(glm::vec3 acceleration);
    void setExternalForce(glm::vec3 force);
    void setFixed(bool fixed);

    // Get
    const glm::vec3* getPosition();
    const glm::vec3* getInitialPosition();
    const glm::vec3* getVelocity();
    const glm::vec3* getAcceleration();

    const int getConnectionCount();
    const glm::vec3* getExternalForce();

    // Get (Boolean Status)
    bool isSurfacePoint();
    bool getFixed();
    glm::vec3 getNaturalLengthV(int link);
    glm::vec3 getActualLengthV(int link);
    glm::vec3 getVelocityDiff(int link);

    // Process
    void addConnection(MassPoint* n);
    void clearAllConnections();


private:
    std::vector <MassPoint*> connectedPoints;
    glm::vec3 initialPos = glm::vec3(0.0);
    glm::vec3 position = glm::vec3(0.0);
    glm::vec3 velocity = glm::vec3(0.0);
    glm::vec3 acceleration = glm::vec3(0.0);
    glm::vec3 externalForce = glm::vec3(0.0);

    bool isFixed = false;
    bool surfacePoint = false;
};

#endif