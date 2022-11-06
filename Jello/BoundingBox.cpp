
#include "BoundingBox.h"

// width is front, depth is z 
BoundingBox::BoundingBox(int width, int height, int depth, glm::vec3 topFrontLeft) {

    glm::vec3 p1 = topFrontLeft;
    glm::vec3 p2 = p1; //topFrontRight
    p2.x = p2.x + width;
    glm::vec3 p3 = topFrontLeft; //bottomFrontLeft
    p3.y = p3.y - height;
    glm::vec3 p4 = p3; //bottomFrontRight
    p4.x = p4.x + width;
    glm::vec3 p5 = p1; //topBackLeft
    p5.z = p5.z - depth; // inside screen is negative z
    glm::vec3 p6 = p2; //topBackRight
    p6.z = p6.z - depth;
    glm::vec3 p7 = p3; //bottomBackLeft
    p7.z = p7.z - depth;
    glm::vec3 p8 = p4; //bottomBackRight
    p8.z = p8.z - depth;

    // front plane 
    Plane* front = new Plane(p1, p2, p3, p4);
    // right plane
    Plane* right = new Plane(p6, p1, p8, p4);
    // left plane
    Plane* left = new Plane(p1, p5, p3, p7);
    // back plane
    Plane* back = new Plane(p5, p6, p7, p8);
    // bottom plane
    Plane* bottom = new Plane(p7, p8, p3, p4);
    // top plane
    Plane* top = new Plane(p1, p2, p5, p6);

    planes.push_back(front);
    planes.push_back(right);
    planes.push_back(left);
    planes.push_back(back);
    planes.push_back(bottom);
    planes.push_back(top);
}

void BoundingBox::render(GLuint modelParameter) {
    for (std::vector<Plane*>::iterator it = this->planes.begin(); it != this->planes.end(); ++it) {
        (*it)->render(modelParameter);
    }
}