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
    // jello cube

    public:

        Cube(); // default constructor
        Cube(int resolution, glm::vec3 position, GLint shader, GLint debug);
        
        // setup
        void setSpringMode(bool structural, bool shear, bool bend);
        void reset();

        // render
        void render(GLuint modelParameter, bool showDiscrete, bool showSpring, bool debugMode);
        int pointSize = 5;
        
        // physics 
        float stiffness = 1500.0f; // store as positive and negate in function so it makes more sense in ImGui
        float damping = 0.5f; // store as positive and negate in function so it makes more sense in ImGui
        float mass = 1.0f;

        // adjustable values
        int resolution = 1;
        // turn on/off springs
        bool structuralSpring;
        bool shearSpring;
        bool bendSpring;
        bool fixedFloor = true;

        // mass points will be stored in discretePoints list but also per faces 
        // faces only store surface points 
        std::vector <MassPoint*> discretePoints{};
        // faces to render triangles
        std::vector <MassPoint*> topFace{};
        std::vector <MassPoint*> bottomFace{};
        std::vector <MassPoint*> rightFace{};
        std::vector <MassPoint*> leftFace{};
        std::vector <MassPoint*> frontFace{};
        std::vector <MassPoint*> backFace{};
        std::vector <std::vector <MassPoint*>*> frontFaces{ &frontFace, &leftFace, &bottomFace }; 
        std::vector <std::vector <MassPoint*>*> backFaces{ &rightFace, &backFace, &topFace }; // different winding order

        void resetAcceleration();
        void setExternalForce(glm::dvec3 force);

    private:
        // render
        GLint shaderProgram;
        GLint debugShaderProgram;
        GLuint VBO, VAO, texVBO, normalVBO; // position, texture, normal
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        std::vector <GLfloat> data{}; // stores vertex positions {x1, y1, z1, x2, y2, z2}
        std::vector <GLfloat> texData{}; // stores UV per vertex {u1, v1, u2, v2}
        std::vector <GLfloat> normalData{}; // stores normal vector xyz per vertex {x1, y1, z1, x2, y2, z2}

        void initArrays();
        void fillDiscretePoints(bool structural, bool shear, bool bend);
        void addConnection(MassPoint**** massPointMap, MassPoint* point, int i, int j, int k);
        void addTriangle(glm::dvec3* pointA, glm::dvec3* pointB, glm::dvec3* pointC);

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

        // attribute locations for debug shaders
        const int debugPosLoc = 0;
};


#endif