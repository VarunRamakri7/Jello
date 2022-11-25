#ifndef __PLATE_H__
#define __PLATE_H__

#include "Plane.h"
#include "MassPoint.h"

class Plate {
public:
    // square plate
    Plate(glm::vec3 center, float size, GLuint debugShader);
    void render(GLuint modelParameter);

    float size = 1;
    Plane* platePlane;
    std::vector <MassPoint*> constraintPoints{};

    void setConstraintPoints(std::vector <MassPoint*> points);
    void setPosition(glm::vec3 position, double timeStep);
};

#endif
