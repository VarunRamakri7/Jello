#ifndef __CUBE_H__
#define __CUBE_H__

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

class Cube {
    public:
        Cube(int resolution);
        void render(GLuint modelParameter, bool showSurface);
        glm::vec4 getModelCoord();
        void getSurface();
        glm::mat4 getModelMatrix();

    private:
        int resolution = 8;
        float scale = 1.0f;
        glm::vec3 position = glm::vec3(0.0f);
        GLuint VBO, VAO;
        std::vector <GLfloat> data {};
        int dataSize = 0;
        void initArrays();
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        void fillDiscretePoints();

        std::vector <float> discretePoints{};
        std::vector <float> surfacePoints{};

        // unit cube (m)
        const std::vector <glm::vec3> initPoints{
            glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f),
            glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f),
        };
};


#endif