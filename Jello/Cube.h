#ifndef __CUBE_H__
#define __CUBE_H__

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include "MassPoint.h"

class Cube {
    public:
        Cube(int resolution);
        void render(GLuint modelParameter, bool showDiscrete);
        glm::vec4 getModelCoord();
        void getSurface();
        glm::mat4 getModelMatrix();
        void setSpringMode(bool structural, bool shear, bool bend);
        void updatePoints();

    private:
        int resolution = 8;
        float scale = 1.0f;
        glm::vec3 position = glm::vec3(0.0f);
        GLuint VBO, VAO;
        std::vector <GLfloat> data {};
        int dataSize = 0;
        void initArrays();
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        void fillDiscretePoints(bool structural, bool shear, bool bend);
        //MassPoint**** massPointMap; // dynamic array of pointers 
        std::vector <MassPoint> discretePoints{};
        //std::vector <MassPoint> surfacePoints{};

        // unit cube (m)
        const std::vector <glm::vec3> initPoints{
            glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f),
            glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f),
        };
};


#endif