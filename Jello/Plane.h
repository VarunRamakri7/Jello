#ifndef __PLANE_H__
#define __PLANE_H__

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <vector>


class Plane {
    public:
        Plane(glm::vec3 pointA, glm::vec3 pointB, glm::vec3 pointC, glm::vec3 pointD, GLuint debugShader);
        
        // render
        void render(GLuint modelParameter);
        glm::dvec3 normal;

        // physics
        glm::dvec3 pointInPlane;
        void setPosition(glm::vec3 pos);
        glm::vec3 getPosition();

    private:
        glm::vec3 pointA; // top left
        glm::vec3 pointB; // top right
        glm::vec3 pointC; // bottom left
        glm::vec3 pointD; // bottom right

        glm::vec3 position = glm::vec3(0.0f);

        // render
        GLuint VBO, VAO;
        std::vector <GLfloat> data{};
        int dataSize = 0;
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        GLuint debugShader;
        int debugPosLoc = 0; // attribute location for vertex position in debug shader

        void initArrays();

};

#endif
