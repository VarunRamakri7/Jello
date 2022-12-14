#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <vector>
#include <omp.h>

#include "Plane.h"
#include "Cube.h" 
#include "BoundingBox.h"

// takes care of the interactions between the objects in scene
// all mass points (physics) related should use double precision

// global variable
extern BoundingBox* boundingBox;

// jelly simulation
glm::dvec3 calculateSpringForce(const double& kh, const glm::dvec3& pointA, const glm::dvec3& pointB, const double restLength);
glm::dvec3 calculateSpringForce(const double& const kh, MassPoint* const pointA, MassPoint* const pointB);

glm::dvec3 calculateDampingForce(const double& kd, const glm::dvec3& pointA, const glm::dvec3& pointB, const glm::dvec3& velA, const glm::dvec3& velB);
glm::dvec3 calculateDampingForce(const double& const kd, MassPoint* const pointA, MassPoint* const pointB);

// internal
void computeSpringAcceleration(const double& stiffness, const double& damping, const double& mass, MassPoint* const currentPoint);
void computeAcceleration(Cube* cube, double timeStep);

// integrators
void integrateEuler(Cube* cube, double timeStep);
void integrateRK4(Cube* cube, double timeStep);

// collision
bool isPointInNegativeSide(const glm::dvec3& point, const Plane& plane);
bool checkCollision(MassPoint* massPoint, BoundingBox* const bbox, glm::dvec3& closesPoint);
void processCollisionResponse(Cube* const cube, MassPoint* const massPoint, const glm::dvec3& closestPoint);
bool isPointInBox(glm::dvec3* const point, BoundingBox* const bbox);

#endif
