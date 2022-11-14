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
    // jello 

    public:

        Cube(); // default constructor
        Cube(int resolution);
        
        // setup
        void setSpringMode(bool structural, bool shear, bool bend);
        void reset();

        // render
        glm::mat4 getModelMatrix();
        void render(GLuint modelParameter, bool showDiscrete);
        int pointSize = 5;
        bool showSpring = false;
        
        // physics 
        double stiffness = -50;
        double damping = -0.5;
        double mass = 1.0;
        double timeStep = 0.005;
        int resolution = 1;

        bool structuralSpring;
        bool shearSpring;
        bool bendSpring;
        bool fixedFloor = true;

        std::vector <MassPoint*> discretePoints{};

        void resetAcceleration();
        void setExternalForce(glm::vec3 force);

    private:
        float scale = 1.0f;
        glm::vec3 position = glm::vec3(0.0f);

        // render
        GLuint VBO, VAO;
        std::vector <GLfloat> data {};
        int dataSize = 0;
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        void initArrays();

        void fillDiscretePoints(bool structural, bool shear, bool bend);

        // unit cube (m)
        const std::vector <glm::vec3> initPoints{
            glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f),
            glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f),
        };

};


#endif