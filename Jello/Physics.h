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

// jelly simulation
glm::dvec3 calculateSpringForce(const double* const kh, MassPoint* const pointA, MassPoint* const pointB);
glm::dvec3 calculateDampingForce(const double* const kd, MassPoint* const pointA, MassPoint* const pointB);
void computeSpringAcceleration(const double* const stiffness, const double* const damping, const double* const mass, MassPoint* const currentPoint);
void computeAcceleration(Cube* cube);

// integrators
void euler(Cube* cube);
void RK4(Cube* cube);

// collision
bool isPointInTriangle(const glm::vec3 point, const glm::vec3 triangleA, const glm::vec3 triangleB, const glm::vec3 triangleC);
bool isSameSide(const glm::vec3 lineA, const glm::vec3 lineB, const glm::vec3 pointA, const glm::vec3 pointB);
bool isPointInNegativeSide(const glm::vec3& point, const Plane& plane);
bool checkCollision(const Cube* cube, const BoundingBox* bbox);
bool isPointInBox(glm::vec3 point, BoundingBox* const bbox);


#endif
