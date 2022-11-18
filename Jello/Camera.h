#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <vector>


class Camera {
public:
    Camera();

    glm::vec3 getPosition();
    glm::vec3 getLookAt();
    glm::mat4 getPV(); // perspective view matrix

    float nearf = 0.01f;
    float farf = 1000.f;

    void setPosition(glm::vec3 position);
    void setLookAt(glm::vec3 lookAt);

private:
    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::vec3 lookAt = glm::vec3(0, 0, 0);
    const glm::vec3 yUp = glm::vec3(0, 1, 0);
    glm::mat4 P = glm::perspective(65.f, 1.f, nearf, farf);
};

#endif