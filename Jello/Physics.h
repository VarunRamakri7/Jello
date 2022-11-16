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
extern std::vector<BoundingBox*> sceneObjs;
//extern Plate* myPlate; // TODO make plane? 

struct collisionPoint {
    MassPoint* mp; // need to keep const cos it already collided ? dont want it to be moving
    glm::dvec3 closestPoint;  // make const?
};

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
bool isPointInTriangle(const glm::vec3 point, const glm::vec3 triangleA, const glm::vec3 triangleB, const glm::vec3 triangleC);
bool isSameSide(const glm::vec3 lineA, const glm::vec3 lineB, const glm::vec3 pointA, const glm::vec3 pointB);
bool isPointInNegativeSide(const glm::dvec3& point, const Plane& plane);
bool checkCollision(MassPoint* massPoint, BoundingBox* const bbox, std::vector<collisionPoint>& collisionPoints);
bool checkCollision(MassPoint* massPoint, Plane* const plane, std::vector<collisionPoint>& collisionPoints);
void processCollisionResponse(Cube* const cube, std::vector<collisionPoint>& collisionPoints);
bool isPointInBox(glm::dvec3* const point, BoundingBox* const bbox);


#endif
