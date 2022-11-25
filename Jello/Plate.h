#ifndef __PLATE_H__
#define __PLATE_H__

#include "Plane.h"
#include "MassPoint.h"

// movable plate that the bottom layer of the jello is constrained to
class Plate {
public:
    // square plate
    Plate(glm::vec3 center, float size, GLuint debugShader);
    void render(GLuint modelParameter);

    float size = 1.0f;
    Plane* platePlane; // geometry
    std::vector <MassPoint*> constraintPoints{}; // points that moving the plate will also move

    void setConstraintPoints(std::vector <MassPoint*> points);
    void setPosition(glm::vec3 position, double timeStep);
};

#endif
