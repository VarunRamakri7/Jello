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

enum drawType {
    DRAWPOINT, DRAWTRI
};

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
        void render(GLuint modelParameter, bool showDiscrete, int drawType);
        int pointSize = 5;
        bool showSpring = false;
        
        // physics 
        double stiffness = -50;
        double damping = -0.5;
        double mass = 1.0;

        int resolution = 1;

        bool structuralSpring;
        bool shearSpring;
        bool bendSpring;
        bool fixedFloor = true;

        std::vector <MassPoint*> discretePoints{};
        // faces
        std::vector <MassPoint*> topFace{};
        std::vector <MassPoint*> bottomFace{};
        std::vector <MassPoint*> rightFace{};
        std::vector <MassPoint*> leftFace{};
        std::vector <MassPoint*> frontFace{};
        std::vector <MassPoint*> backFace{};
        std::vector <std::vector <MassPoint*>*> faces{&topFace, &bottomFace , &rightFace, &leftFace, &frontFace , &backFace };
       
        void resetAcceleration();
        void setExternalForce(glm::vec3 force);

    //private:
        // render
        GLuint VBO, VAO, texVBO, normalVBO; // position, texture, normal
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        void initArrays();
        void fillDiscretePoints(bool structural, bool shear, bool bend);
        void addConnection(MassPoint**** massPointMap, MassPoint* point, int i, int j, int k);

        float scale = 1.0f;
        glm::vec3 position = glm::vec3(0.0f);
        // unit cube (m)
        const std::vector <glm::vec3> initPoints{
            glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f),
            glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f),
        };

        // attribute locations for shaders
        const int posLoc = 0;
        const int texCoordLoc = 1;
        const int normalLoc = 2;

};


#endif