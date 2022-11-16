#ifndef __PLATE_H__
#define __PLATE_H__

#include "Plane.h"
#include "MassPoint.h"

class Plate {
public:
    // square plate
    Plate(glm::vec3 center, double size);
    void render(GLuint modelParameter); // TODO add view mode 
    double size = 1;
    Plane* platePlane;
    void offsetPosition(glm::dvec3 offset, double timeStep);
    void setConstraintPoints(std::vector <MassPoint*> points);
    glm::vec3 position;

    std::vector <MassPoint*> constraintPoints{};
    void setPosition(glm::vec3 position);

    void shake(glm::vec2 change, double speed);

    const double maxShake = 0.5;
};

#endif
