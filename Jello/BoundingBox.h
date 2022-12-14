#ifndef __BOUNDINGBOX_H__
#define __BOUNDINGBOX_H__

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <vector>

#include "Plane.h"

class BoundingBox {
    public:
        BoundingBox(int width, int height, int depth, glm::vec3 topFrontLeft, GLuint debugShader);
        void render(GLuint modelParameter);

        double minX;
        double minY;
        double minZ;
        double maxX;
        double maxY;
        double maxZ;

        std::vector <Plane*> planes{}; // 6 sides / planes
};

#endif
