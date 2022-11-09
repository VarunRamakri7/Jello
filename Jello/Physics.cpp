#include "Physics.h"
#include <iostream>

// COLLISION DETECTION

/**
 * checks if given point is in triangle formed by the 3 given points
 * @param glm::vec3 point
 * @param glm::vec3 triangleA - vertex A of triangle
 * @param glm::vec3 triangleB - vertex B of triangle
 * @param glm::vec3 triangleC - vertex C of triangle
 * @return bool - true if in the triangle
 */
bool isPointInTriangle(glm::vec3 point, glm::vec3 triangleA, glm::vec3 triangleB, glm::vec3 triangleC) {
    if (isSameSide(point, triangleA, triangleB, triangleC) && isSameSide(point, triangleB, triangleA, triangleC) && isSameSide(point, triangleC, triangleA, triangleB)) {
        return true;
    }
    return false;
}

bool isSameSide(glm::vec3 lineA, glm::vec3 lineB, glm::vec3 pointA, glm::vec3 pointB) {
    glm::vec3 crossA = glm::cross(pointB - pointA, lineA - pointA);
    glm::vec3 crossB = glm::cross(pointB - pointA, lineB - pointA);

    if (glm::dot(crossA, crossB) >= 0) {
        return true;
    }
    return false;
}

bool isPointInNegativeSide(const glm::dvec3& point, const Plane& plane){

    // ax + by + cz - (ax1 + by1+ cz1) = 0
    // (normal . point) - (normal . pointinplane) = 0
    // F(x,y,z)>0 on one side of the plane and F(x,y,z)<0 on the other
    // return pl.a * pt.x + pl.b * pt.y + pl.c * pt.z + pl.d >= 0;
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
// NOTE THIS WILL ONLY WORK FOR BOUDNING BOX AND NOT OTHER CUBES INSIDE THE BOX
// BECAUSE THERES NO BOUNDARIES< ONLY CHECKING IF INSIDE
glm::dvec3 computeClosestPoint(const glm::dvec3& point, Plane plane)
{
    // TO DO define the point and vectors as glm::dvec3? 

    // get normal

    // create a ray with origin at pt, with -n direction and parameter t: r = pt - nt
    // compute t when ray intersect with plane ax + by + cz + d = 0
    // t = (<n,pt>+d)/<n,n>
    double length2 = glm::dot(plane.normal, plane.normal); // length2
    double dot0 = glm::dot(plane.normal, point);

    double t = (dot0 + glm::dot(plane.normal, plane.pointInPlane)) / length2;

    // insert t back into ray equation we get intersection point : p1 = pt - tn
    return plane.normal * (-t) + point;
}

bool checkCollision(MassPoint* massPoint, BoundingBox* const bbox, std::vector<collisionPoint> &collisionPoints) {
    // for mass points in cube, check if in boundingbox
    // if not inside, check if colliding 

    glm::dvec3* const pos = massPoint->getPosition(); // already world space
    //glm::dvec4 p = glm::dvec4(*pos, 1.0) * cube->getModelMatrix();
    //if (i == 0) {
    //    //std::cout << "P :" << p.x << ", " << p.y << ", " << p.z << std::endl;
    //    //std::cout << "P :" << pos->x << ", "<< pos->y << ", " << pos->z << std::endl;
    //    if (isPointInBox(pos, bbox)) {
    //        //std::cout << "P in " << std::endl;
    //    }
    //    else {
    //        //std::cout << "P out " << std::endl;
    //    }
    //}
    if (!isPointInBox(pos, bbox)) {
        // collide 
        // each plane in box, check for collision
        //std::cout << "out!" << std::endl;
        for (int p = 0; p < 6; p++) {
            if (isPointInNegativeSide(*pos, *bbox->planes[p])) {
                // find intersection point in plane 
                //std::cout << "collide!" << std::endl;

                // store mass point, closest point of collision, list of collision springs to process 
                struct collisionPoint cp;
                cp.closestPoint = computeClosestPoint(*pos, *bbox->planes[p]);
                cp.mp = massPoint;
                collisionPoints.push_back(cp);
                // process for all corners that maybe intersecting, so maybe it'll work 
            }
        }
    }
    
    return collisionPoints.size() != 0;
}

void processCollisionResponse(Cube* const cube, std::vector<collisionPoint>& collisionPoints) {
    if (collisionPoints.size() == 0) // no collision
        return;

    // create a spring for each collision

    // compute acceleration for each collision
    for (const auto& s : collisionPoints)
    {

        // compute elastic force and damping
        // TODO collosion has different stiffness and damping value 
        glm::dvec3 springForce = calculateSpringForce(cube->stiffness , *(s.mp->getPosition()), s.closestPoint, 0.0);
        glm::dvec3 dampingForce = calculateDampingForce(cube->damping * 50.0, *(s.mp->getPosition()), s.closestPoint, *(s.mp->getVelocity()), glm::dvec3(0.0));

        // F = ma -> a = F / m 
        // update force on current mass point that collided
        s.mp->addAcceleration((springForce + dampingForce) / cube->mass);
    }
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
void computeAcceleration(Cube* cube) {
    // TODO pass in list of objects in scene or use as extern variable 
    // reset accumulated acceleration to 0
    cube->resetAcceleration();

    std::vector<collisionPoint> collisionPoints;

    // goes through all masspoints
    for (int i = 0; i < cube->discretePoints.size(); i++) {
        MassPoint* currentPoint = cube->discretePoints[i];

        // calculate spring acceleration for each mass points
        computeSpringAcceleration(cube->stiffness, cube->damping, cube->mass, currentPoint);

        // check if each point collides with any objects in the scene
        for (const auto& s : sceneObjs) {
            checkCollision(currentPoint, s, collisionPoints);
        }

        // external forces
        // TODO  division is expensive? 
        glm::dvec3 externalAcc = (*currentPoint->getExternalForce()) / cube->mass;
        currentPoint->addAcceleration(externalAcc);
    }

    // compute force from collision as response
    processCollisionResponse(cube, collisionPoints);
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

    return kh * (currentLength - restLength) * (L / currentLength);
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

    return kd * (glm::dot(velocityDiff, L) / lengthL) * (L / lengthL);
}

// INTEGRATORS - numerical solution to analytical problems

/**
 * performs one step Euler integration (may explode if time step is too big), 
 * approximating the acceleration
 * velocity = dx/dt (change in position over change in time)
 * acceleration = dv/dt (change in velocity over change in time)
 * @param Cube* const cube - constant pointer to a cube
 */
void euler(Cube* const cube) {
    
    // compute accumulated acceleration of mass points in cube
    computeAcceleration(cube);

    // integrate 
    for (int i = 0; i < cube->discretePoints.size(); i++) {
        MassPoint* currentPoint = cube->discretePoints[i];

        if (currentPoint->isFixed() == true) {
            // stays the same
            continue;
        }

        // one step euler
        // Velocity
        glm::dvec3 vel = *currentPoint->getVelocity() + (*currentPoint->getAcceleration() * cube->timeStep);
        currentPoint->setVelocity(vel);

        // Position
        glm::dvec3 pos = *currentPoint->getPosition() + (*currentPoint->getVelocity() * cube->timeStep);
        currentPoint->setPosition(pos);
    }
}

/**
 * performs Runge-Kutta 4th order integration (more stable but requires smaller time step than euler),
 * approximating the change in position and velocity using doubles
 * @param Cube* const cube - constant pointer to a cube
 */

void RK4(Cube* cube) {
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
    computeAcceleration(cube);

    // integrate
    for (int i = 0; i < cube->discretePoints.size(); i++) {
        MassPoint* currentPoint = cube->discretePoints[i];
        MassPoint* bufferPoint = buffer.discretePoints[i];

        // dx/dt = F(t, x)
        // 1st step: k1 =  F(t0, x0)

        // Position
        glm::dvec3 dVel = *currentPoint->getVelocity() * cube->timeStep;
        // Velocity
        glm::dvec3 dAcc = *currentPoint->getAcceleration() * cube->timeStep;

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

    computeAcceleration(&buffer);

    for (int i = 0; i < cube->discretePoints.size(); i++) {
        MassPoint* currentPoint = cube->discretePoints[i];
        MassPoint* bufferPoint = buffer.discretePoints[i];

        // 2nd step: k2 = F(t + dt/2, x + h * k1/2) 
        // Position
        glm::dvec3 dVel = *bufferPoint->getVelocity()* cube->timeStep;
        F2p.push_back(dVel);
        
        // Velocity
        glm::dvec3 dAcc = *bufferPoint->getAcceleration() * cube->timeStep;
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

    computeAcceleration(&buffer);

    for (int i = 0; i < cube->discretePoints.size(); i++) {
        MassPoint* currentPoint = cube->discretePoints[i];
        MassPoint* bufferPoint = buffer.discretePoints[i];

        // 3rd step: k3 = F(t + dt/2, x + h * k2/2) 
        // Position
        glm::dvec3 dVel = *bufferPoint->getVelocity() * cube->timeStep;
        F3p.push_back(dVel);

        // Velocity
        glm::dvec3 dAcc = *bufferPoint->getAcceleration() * cube->timeStep;
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

    computeAcceleration(&buffer);

    for (int i = 0; i < cube->discretePoints.size(); i++) {
        MassPoint* currentPoint = cube->discretePoints[i];
        MassPoint* bufferPoint = buffer.discretePoints[i];

        // 4th step: k4 = F(t + dt, x + h * k3) 
        // Position
        glm::dvec3 dVel = *bufferPoint->getVelocity() * cube->timeStep;
        F4p.push_back(dVel);

        // Velocity
        glm::dvec3 dAcc = *bufferPoint->getAcceleration() * cube->timeStep;
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