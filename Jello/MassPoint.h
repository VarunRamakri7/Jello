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
        MassPoint(glm::dvec3 position, bool isSurfacePoint);
        MassPoint(glm::dvec3 position, glm::dvec3 velocity);

        MassPoint(){}; // default constructor 
        ~MassPoint(){};

        // Set
        void setPosition(glm::dvec3 position);
        void setVelocity(glm::dvec3 velocity);
        void setAcceleration(glm::dvec3 acceleration);
        void setExternalForce(glm::dvec3 force);
        void setFixed(bool fixed);

        void addAcceleration(glm::dvec3 acc);

        // Get
        glm::dvec3* getPosition();
        glm::dvec3* getInitialPosition();
        glm::dvec3* getVelocity();
        glm::dvec3* getAcceleration();
        MassPoint* getConnection(int link);
        int getConnectionCount();
        glm::dvec3* getExternalForce();

        // Get (Boolean Status)
        bool isSurfacePoint();
        bool isFixed();

        // Process
        void addConnection(MassPoint* n);
        void clearAllConnections();


    private:
        std::vector <MassPoint*> connectedPoints;
        glm::dvec3 initialPos = glm::dvec3(0.0);
        glm::dvec3 position = glm::dvec3(0.0);
        glm::dvec3 velocity = glm::dvec3(0.0);
        glm::dvec3 acceleration = glm::dvec3(0.0);
        glm::dvec3 externalForce = glm::dvec3(0.0);

        bool fixed = false;
        bool surfacePoint = false;
};


#endif