#include "Physics.h"
#include <iostream>

// COLLISION 
bool isPointInNegativeSide(const glm::dvec3& point, const Plane& plane){
    return glm::dot(plane.normal, point) - glm::dot(plane.normal, plane.pointInPlane) < 0;
}

bool isPointInBox(glm::dvec3* const point, BoundingBox* const bbox) {
    return ((point->x >= bbox->minX && point->x <= bbox->maxX)
        && (point->y >= bbox->minY && point->y <= bbox->maxY)
        && (point->z >= bbox->minZ && point->z <= bbox->maxZ));
}

/**
 * compute the closest point from plane to a point
 * @param pt - point
 * @param pl - plane
 * @return closest point to point in plane
 */
// NOTE: this will only work for boudning box and not other cubes inside the box
// because theres no boundaries > only checking if inside
glm::dvec3 computeClosestPoint(const glm::dvec3& point, Plane plane)
{
    // create a ray with origin at pt, with -n direction and parameter t: r = pt - nt
    // compute t when ray intersect with plane ax + by + cz + d = 0
    // t = (<n,pt>+d)/<n,n>
    double length2 = glm::dot(plane.normal, plane.normal); 
    double dot0 = glm::dot(plane.normal, point);

    double t = (dot0 + glm::dot(plane.normal, plane.pointInPlane)) / length2;

    // insert t back into ray equation we get intersection point : p1 = pt - tn
    return plane.normal * (-t) + point;
}

bool checkCollision(MassPoint* massPoint, BoundingBox* const bbox, glm::dvec3& closesPoint) {
    // for mass points in cube, check if in boundingbox
    // if not inside, check if colliding 
    // only gives one collision point per point, 
    // if point hits two planes at once (ie: corner) it'll process one at a time

    glm::dvec3* const pos = massPoint->getPosition(); // already world space

    if (!isPointInBox(pos, bbox)) {
        // collide 
        // check for collision with each plane in box
        for (int p = 0; p < 6; p++) {
            if (isPointInNegativeSide(*pos, *bbox->planes[p])) {
                // find intersection point in plane 
                // store mass point, closest point of collision, list of collision springs to process 
                closesPoint = computeClosestPoint(*pos, *bbox->planes[p]);
                return true;
            }
        }
    }

    return false;
}

void processCollisionResponse(Cube* const cube, MassPoint* const massPoint, const glm::dvec3& closestPoint) {
    // compute elastic force and damping
    glm::dvec3 springForce = calculateSpringForce(cube->stiffness, *(massPoint->getPosition()), closestPoint, 0.0);
    glm::dvec3 dampingForce = calculateDampingForce(cube->damping * 50.0, *(massPoint->getPosition()), closestPoint, *(massPoint->getVelocity()), glm::dvec3(0.0));

    // F = ma -> a = F / m 
    // update force on current mass point that collided
    massPoint->addAcceleration((springForce + dampingForce) / double(cube->mass));
}

// PHYSICS

/**
 * computes spring force and acceleration per point 
 * @param const double* const  stiffness - constant pointer to constant double stiffness 
 * @param const double* const  damping - vertex A of triangle
 * @param const double* const  mass - vertex B of triangle
 * @param MassPoint* const currentPoint - constant pointer to current MassPoint
 */
void computeSpringAcceleration(const double& stiffness, const double& damping, const double& mass, MassPoint* const currentPoint) {
    // gets all connected masspoints (depends on the spring types enabled)
    for (int c = 0; c < currentPoint->getConnectionCount(); c++) {
        MassPoint* pointB = currentPoint->getConnection(c);

        glm::dvec3 s = calculateSpringForce(stiffness, currentPoint, pointB);
        glm::dvec3 d = calculateDampingForce(damping, currentPoint, pointB);
        glm::dvec3 force = s + d;
        
        // F = ma -> a = F / m 
        // update force on current mass point
        currentPoint->addAcceleration(force / mass);
        // update opposite forces on B 
        pointB->addAcceleration(-force / mass);
    }
}

/**
 * computes accumulated acceleration for all masspoints in cube
 * @param Cube* cube
 */
void computeAcceleration(Cube* cube, double timeStep) {
    // reset accumulated acceleration to 0
    cube->resetAcceleration();

    // goes through all masspoints
    #pragma omp parallel for shared(boundingBox)
    for (int i = 0; i < cube->discretePoints.size(); i++) {
        MassPoint* currentPoint = cube->discretePoints[i];

        // calculate spring acceleration for each mass points
        computeSpringAcceleration(cube->stiffness, cube->damping, cube->mass, currentPoint);

        // check if each point collides with bounding box
        glm::dvec3 closestPoint;
        if (checkCollision(currentPoint, boundingBox, closestPoint)) {
            // if collided, process collision response 
            processCollisionResponse(cube, currentPoint, closestPoint);
        }

        // external forces
        // TODO  division is expensive? 
        glm::dvec3 externalAcc = (*currentPoint->getExternalForce()) / double(cube->mass);
        currentPoint->addAcceleration(externalAcc);
    }
}

/**
 * computes Hooks law in 3D (spring force)
 * @param const double* const kh - hook's constant = stiffness (should be negative)
 * @param MassPoint* pointA - current mass point
 * @param MassPoint* pointB - neighboring mass point
 * @return glm::dvec3 - spring force
 */
glm::dvec3 calculateSpringForce(const double& const kh, MassPoint* pointA, MassPoint* pointB) {
    // F = kh * (|L| - R) * (L / |L|)
    // vector from start to end = end - start
    glm::dvec3 L = *(pointA->getPosition()) - *(pointB->getPosition()); // vector from current neighbor (pointB) to point (pointA)
    double currentLength = glm::length(L);
    double restLength = glm::length(*(pointA->getInitialPosition()) - *(pointB->getInitialPosition()));

    return calculateSpringForce(kh, *(pointA->getPosition()), *(pointB->getPosition()), restLength);
}
glm::dvec3 calculateSpringForce(const double& kh, const glm::dvec3& pointA, const glm::dvec3& pointB, const double restLength) {
    // F = kh * (|L| - R) * (L / |L|)
    // vector from start to end = end - start

    glm::dvec3 L = pointA - pointB; // vector from current neighbor (pointB) to point (pointA)
    double currentLength = glm::length(L);
    //double restLength = glm::length(*(pointA->getInitialPosition()) - *(pointB->getInitialPosition()));

    // stiffness (kh) is a negative force
    return (-1.0) * kh * (currentLength - restLength) * (L / currentLength);
}

/**
 * computes damping force in 3D 
 * @param const double* const kd - damping constant (should be negative)
 * @param MassPoint* pointA - current mass point
 * @param MassPoint* pointB - neighboring mass point
 * @return glm::dvec3 - damping force
 */
glm::dvec3 calculateDampingForce(const double& const kd, MassPoint* const pointA, MassPoint* const pointB) {
    return calculateDampingForce(kd, *(pointA->getPosition()), *(pointB->getPosition()), *(pointA->getVelocity()), *(pointB->getVelocity()));
}
glm::dvec3 calculateDampingForce(const double& kd, const glm::dvec3& pointA, const glm::dvec3& pointB, const glm::dvec3& velA, const glm::dvec3& velB) {
    // F = kd * ((Va - Vb) dot L ) / |L| * (L / |L|)
    glm::dvec3 L = pointA - pointB; // vector from current neighbor (pointB) to point (pointA)
    double lengthL = glm::length(L); // current distance between pointA and pointB
    glm::dvec3 velocityDiff = velA - velB;

    // damping value (kh) is a negative force
    return (-1.0) * kd * (glm::dot(velocityDiff, L) / lengthL) * (L / lengthL);
}


// INTEGRATORS - numerical solution to analytical problems

/**
 * performs one step Euler integration (may explode if time step is too big), 
 * approximating the acceleration
 * velocity = dx/dt (change in position over change in time)
 * acceleration = dv/dt (change in velocity over change in time)
 * @param Cube* const cube - constant pointer to a cube
 */
void integrateEuler(Cube* const cube, double timeStep) {
    
    // compute accumulated acceleration of mass points in cube
    computeAcceleration(cube, timeStep);

    // integrate 
    #pragma omp parallel for
    for (int i = 0; i < cube->discretePoints.size(); i++) {
        MassPoint* currentPoint = cube->discretePoints[i];
        if (currentPoint->isFixed() == true) {
            // stays the same
            continue;
        }

        // one step euler
        // Velocity
        glm::dvec3 vel = *currentPoint->getVelocity() + (*currentPoint->getAcceleration() * timeStep);
        currentPoint->setVelocity(vel);

        // Position
        glm::dvec3 pos = *currentPoint->getPosition() + (*currentPoint->getVelocity() * timeStep);
        currentPoint->setPosition(pos);
    }
}

/**
 * performs Runge-Kutta 4th order integration (more stable but requires smaller time step than euler),
 * approximating the change in position and velocity using doubles
 * @param Cube* const cube - constant pointer to a cube
 */

void integrateRK4(Cube* cube, double timeStep) {
    // needs 4 integration for both point and velocity 
    // approximate differential equations

    // make 4 arrays of glm::dvec3 for differentiated position and velocity
    std::vector <glm::dvec3> F1p{}; // first step for position
    std::vector <glm::dvec3> F2p{}; // second step for position
    std::vector <glm::dvec3> F3p{}; // third step for position
    std::vector <glm::dvec3> F4p{}; // fourth step for position

    std::vector <glm::dvec3> F1v{}; // first step for velocity
    std::vector <glm::dvec3> F2v{}; // second step for velocity
    std::vector <glm::dvec3> F3v{}; // third step for velocity
    std::vector <glm::dvec3> F4v{}; // fourth step for velocity

    Cube buffer = *cube; // make a copy of cube

    // compute accumulated acceleration for all mass points in cube
    computeAcceleration(cube, timeStep);

    // integrate
    #pragma omp parallel for
    for (int i = 0; i < cube->discretePoints.size(); i++) {
        MassPoint* currentPoint = cube->discretePoints[i];
        MassPoint* bufferPoint = buffer.discretePoints[i];

        // dx/dt = F(t, x)
        // 1st step: k1 =  F(t0, x0)
        glm::dvec3 dVel = *currentPoint->getVelocity() * timeStep;
        // Velocity
        glm::dvec3 dAcc = *currentPoint->getAcceleration() * timeStep;

        F1p.push_back(dVel);
        F1v.push_back(dAcc);
        
        // store for second step: 
        glm::dvec3 m_pos = *currentPoint->getPosition() + (dVel * 0.5);
        glm::dvec3 m_vel = *currentPoint->getVelocity() + (dAcc * 0.5);
        
        if (currentPoint->isFixed() == true) {
            // no change
            bufferPoint->setPosition(*currentPoint->getPosition());
            bufferPoint->setVelocity(*currentPoint->getVelocity());
        }
        else {
            bufferPoint->setPosition(m_pos);
            bufferPoint->setVelocity(m_vel);
        }
    }

    computeAcceleration(&buffer, timeStep);

    #pragma omp parallel for
    for (int i = 0; i < cube->discretePoints.size(); i++) {
        MassPoint* currentPoint = cube->discretePoints[i];
        MassPoint* bufferPoint = buffer.discretePoints[i];

        // 2nd step: k2 = F(t + dt/2, x + h * k1/2) 
        // Position
        glm::dvec3 dVel = *bufferPoint->getVelocity()* timeStep;
        F2p.push_back(dVel);
        
        // Velocity
        glm::dvec3 dAcc = *bufferPoint->getAcceleration() * timeStep;
        F2v.push_back(dAcc);

        // store for 3rd step
        glm::dvec3 m_pos = *currentPoint->getPosition() + (dVel * 0.5);
        glm::dvec3 m_vel = *currentPoint->getVelocity() + (dAcc * 0.5);
       
        if (currentPoint->isFixed() == true) {
            // no change
            bufferPoint->setPosition(*currentPoint->getPosition());
            bufferPoint->setVelocity(*currentPoint->getVelocity());
        }
        else {
            bufferPoint->setPosition(m_pos);
            bufferPoint->setVelocity(m_vel);
        }
    }

    computeAcceleration(&buffer, timeStep);

    #pragma omp parallel for
    for (int i = 0; i < cube->discretePoints.size(); i++) {
        MassPoint* currentPoint = cube->discretePoints[i];
        MassPoint* bufferPoint = buffer.discretePoints[i];

        // 3rd step: k3 = F(t + dt/2, x + h * k2/2) 
        // Position
        glm::dvec3 dVel = *bufferPoint->getVelocity() * timeStep;
        F3p.push_back(dVel);

        // Velocity
        glm::dvec3 dAcc = *bufferPoint->getAcceleration() * timeStep;
        F3v.push_back(dAcc);

        // store for 4th step
        glm::dvec3 m_pos = *currentPoint->getPosition() + dVel;
        glm::dvec3 m_vel = *currentPoint->getVelocity() + dAcc;

        if (currentPoint->isFixed() == true) {
            // no change
            bufferPoint->setPosition(*currentPoint->getPosition());
            bufferPoint->setVelocity(*currentPoint->getVelocity());
        }
        else {
            bufferPoint->setPosition(m_pos);
            bufferPoint->setVelocity(m_vel);
        }
    }

    computeAcceleration(&buffer, timeStep);

    #pragma omp parallel for
    for (int i = 0; i < cube->discretePoints.size(); i++) {
        MassPoint* currentPoint = cube->discretePoints[i];
        MassPoint* bufferPoint = buffer.discretePoints[i];

        // 4th step: k4 = F(t + dt, x + h * k3) 
        // Position
        glm::dvec3 dVel = *bufferPoint->getVelocity() * timeStep;
        F4p.push_back(dVel);

        // Velocity
        glm::dvec3 dAcc = *bufferPoint->getAcceleration() * timeStep;
        F4v.push_back(dAcc);

        if (currentPoint->isFixed() == true) {
            // no change
            continue;
        }

        // dx = dt * (k1 + 2 * k2 + 2* k3 + k4)/6
        // x = x + dx
        glm::dvec3 p = *currentPoint->getPosition() + ((F1p[i] + (F2p[i] * 2.0) + (F3p[i] * 2.0) + F4p[i]) / 6.0);
        glm::dvec3 v = *currentPoint->getVelocity() + ((F1v[i] + (F2v[i] * 2.0) + (F3v[i] * 2.0) + F4v[i]) / 6.0);
        currentPoint->setVelocity(v);
        currentPoint->setPosition(p);
    }

}