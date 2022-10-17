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
    MassPoint(){};
    ~MassPoint(){};

    // Set
    void setPosition(glm::vec3 position);
    void setVelocity(glm::vec3 velocity);
    void setAcceleration(glm::vec3 acceleration);
    //void setExternalForce(mat& f);
    /*void setPositionImposed(bool f);
    void setFixed(bool f);
    void setSurfaceNode(bool f);
    void setRadius(double r);*/

    // Get
    const glm::vec3* getPosition();
    //const mat* getConnectedNodePosition(int node);
    const glm::vec3* getVelocity();
    const glm::vec3* getAcceleration();
    //const mat* getExternalForce();
    /*void getNaturalLength(int link, mat& naturalLength);
    void getActualLength(int linkmat, mat& actualLength);
    double getNormNaturalLength(int link);
    double getNormActualLength(int link);
    int getNumberofConnectedNodes();
    CNode* getConnectedNodes(int index);
    double getRadius();*/

    // Get (Boolean Status)
    bool isSurfacePoint();

    // Process
    void addConnection(MassPoint* n);
    void clearAllConnections();


private:
    std::vector <MassPoint*> connectedPoints;
    glm::vec3 initialPos;
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    //mat mExternalForce;

    bool isPositionImposed;
    bool isFixed;
    bool surfacePoint;
};

#endif