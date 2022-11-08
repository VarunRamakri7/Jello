#ifndef __CUBE_H__
#define __CUBE_H__

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <vector>

#include "MassPoint.h"

class Cube {
    public:

        Cube(int resolution);
        Cube();

        void render(GLuint modelParameter, bool showDiscrete);
        glm::vec4 getModelCoord();
        void getSurface();
        glm::mat4 getModelMatrix();
        void setSpringMode(bool structural, bool shear, bool bend);
        void updatePoints(float time);
        void setExternalForce(glm::vec3 force);
        void setFixedFloor(bool set);
        bool getFixedFloor();
        std::vector <MassPoint*>* getMassPoints(); // get constant pointer to the mass points vector 
        void resetAcceleration();
        std::vector <MassPoint*> discretePoints{};
        //physics
        double stiffness = -50;
        double damping = -0.5;
        double mass = 1.0;
        double timeStep = 0.005;
        int resolution = 8;

    private:
        float scale = 1.0f;
        glm::vec3 position = glm::vec3(0.0f);
        GLuint VBO, VAO;
        std::vector <GLfloat> data {};
        int dataSize = 0;
        void initArrays();
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        void fillDiscretePoints(bool structural, bool shear, bool bend);
        //MassPoint**** massPointMap; // dynamic array of pointers 
        
        //std::vector <MassPoint> surfacePoints{};

        bool structuralSpring;
        bool shearSpring;
        bool bendSpring;
        bool fixedFloor = true;

        // unit cube (m)
        const std::vector <glm::vec3> initPoints{
            glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f),
            glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f),
        };

        
        
};


#endif