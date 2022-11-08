#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <vector>

#include "Plane.h"
#include "Cube.h" 
#include "BoundingBox.h"

// takes care of the interactions between the objects in scene

bool isPointInTriangle(const glm::vec3 point, const glm::vec3 triangleA, const glm::vec3 triangleB, const glm::vec3 triangleC);
bool isSameSide(const glm::vec3 lineA, const glm::vec3 lineB, const glm::vec3 pointA, const glm::vec3 pointB);
bool isPointInNegativeSide(const glm::vec3& point, const Plane& plane);
bool checkCollision(const Cube* cube, const BoundingBox* bbox);
bool isPointInBox(glm::vec3 point, BoundingBox* const bbox);
glm::dvec3 calculateSpringForce(const double kh, MassPoint* pointA, MassPoint* pointB);
glm::dvec3 calculateDampingForce(const double kd, MassPoint* pointA, MassPoint* pointB);
void computeSpringAcceleration(const double stiffness, const double damping, const double mass, MassPoint* currentPoint);
void computeAcceleration(const double stiffness, const double damping, const double mass, Cube* cube);
void euler(Cube* cube);
void RK4(Cube* cube);

#endif
