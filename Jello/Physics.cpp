#include "Physics.h"
#include <iostream>

// collision detection
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

bool isPointInNegativeSide(const glm::vec3& point, const Plane& plane){

    // ax + by + cz - (ax1 + by1+ cz1) = 0
    // (normal . point) - (normal . pointinplane) = 0
    // F(x,y,z)>0 on one side of the plane and F(x,y,z)<0 on the other
    // return pl.a * pt.x + pl.b * pt.y + pl.c * pt.z + pl.d >= 0;
    return glm::dot(plane.normal, point) - glm::dot(plane.normal, plane.pointInPlane) < 0;
    // return false;
}

bool isPointInBox(const glm::vec3 point, BoundingBox* const bbox) {
    return ((point.x >= bbox->minX && point.x <= bbox->maxX)
        && (point.y >= bbox->minY && point.y <= bbox->maxY)
        && (point.z >= bbox->minZ && point.z <= bbox->maxZ));
}

bool checkCollision(Cube* const cube, BoundingBox* const bbox) {
    // for mass points in cube, check if in boundingbox
    // if not inside, check if colliding 
    const std::vector <MassPoint*>& vecRef = *cube->getMassPoints();
    for (int i = 0; i < vecRef.size(); i++) {
        MassPoint* massPoint = vecRef[i];
        const glm::dvec3* pos = massPoint->getPosition();
        if (isPointInBox(*pos, bbox)) {
            // collide 
            // each plane in box, check for collision
            for (int p = 0; p < 6; p++) {
                if (isPointInNegativeSide(*pos, *bbox->planes[p])) {
                    // find intersection point in plane 
                    std::cout << "collide!" << std::endl;
                    return true;
                }
            }
        }
    }
    return false;
}

// give jello a structure
void computeSpringAcceleration(const double stiffness, const double damping, const double mass, MassPoint* currentPoint) {

    for (int c = 0; c < currentPoint->getConnectionCount(); c++) {
        MassPoint* pointB = currentPoint->getConnection(c);

        glm::dvec3 s = calculateSpringForce(stiffness, currentPoint, pointB);
        glm::dvec3 d = calculateDampingForce(damping, currentPoint, pointB);
        glm::dvec3 force = s + d;

        // update opposite forces on B 
        pointB->addAcceleration(-force / mass);
        currentPoint->addAcceleration(force / mass);
    }
}


void computeAcceleration(const double stiffness, const double damping, const double mass, Cube* cube) {
    
    // compute forces at each node 
    cube->resetAcceleration();
    
    // spring acceleration

    for (int i = 0; i < cube->discretePoints.size(); i++) {
        MassPoint* currentPoint = cube->discretePoints[i];

        // need to compute all acceration, then get stored acceleration to do velocity 
        computeSpringAcceleration(stiffness, damping, mass, currentPoint);

        // external forces
        // division is expensive? 
        glm::dvec3 externalAcc = (*currentPoint->getExternalForce()) / mass;
        currentPoint->addAcceleration(externalAcc);
    }
}

glm::dvec3 calculateSpringForce(const double kh, MassPoint* pointA, MassPoint* pointB) {
    // hooks law in 3d
    // kh is already negative
    // F = kh * (|L| - R) * (L / |L|)
    // end - start
    glm::dvec3 L = *(pointA->getPosition()) - *(pointB->getPosition()); // vector from current neighbor (pointB) to point (pointA)
    double currentLength = glm::length(L);
    double restLength = glm::length(*(pointA->getInitialPosition()) - *(pointB->getInitialPosition()));
    //glm::dvec3 Lnorm = glm::normalize(L);
    //glm::dvec3 Llen =  L / currentLength;
    return kh * (currentLength - restLength) * (L / currentLength);
}

// kd is already negative 
glm::dvec3 calculateDampingForce(double kd, MassPoint* pointA, MassPoint* pointB) {
    // F = kd * ((Va - Vb) dot L ) / |L| * (L / |L|)
    glm::dvec3 L = *pointA->getPosition() - *pointB->getPosition(); // vector from current neighbor (pointB) to point (pointA)
    double lengthL = glm::length(L); // current distance between pointA and pointB
    glm::dvec3 velocityDiff = *pointA->getVelocity() - *pointB->getVelocity();

    return kd * (glm::dot(velocityDiff, L) / lengthL) * (L / lengthL); // is glm::normalize(L) the same? 

}

//integrator
void euler(Cube* cube) {
    
    // doubles precision
    computeAcceleration(cube->stiffness, cube->damping, cube->mass, cube);

    for (int i = 0; i < cube->discretePoints.size(); i++) {
        MassPoint* currentPoint = cube->discretePoints[i];

        if (currentPoint->getFixed() == true) {
            continue;
        }

        // one step euler integration 
            // Velocity
        glm::dvec3 vel = *currentPoint->getVelocity() + (*currentPoint->getAcceleration() * cube->timeStep);
        currentPoint->setVelocity(vel);

        // Position
        glm::dvec3 pos = *currentPoint->getPosition() + (*currentPoint->getVelocity() * cube->timeStep);
        currentPoint->setPosition(pos);
    }
}

void RK4(Cube* cube) {
    // needs 4 integration for both point and velocity 
    // approximate differential equations

    // make 4 arrays of glm::dvec3 for position and velocity integration 
    std::vector <glm::dvec3> F1p{};
    std::vector <glm::dvec3> F2p{};
    std::vector <glm::dvec3> F3p{};
    std::vector <glm::dvec3> F4p{};

    std::vector <glm::dvec3> F1v{};
    std::vector <glm::dvec3> F2v{};
    std::vector <glm::dvec3> F3v{};
    std::vector <glm::dvec3> F4v{};

    Cube buffer = *cube;

    computeAcceleration(cube->stiffness, cube->damping, cube->mass, cube);

    for (int i = 0; i < cube->discretePoints.size(); i++) {
        MassPoint* currentPoint = cube->discretePoints[i];
        MassPoint* bufferPoint = buffer.discretePoints[i];

        // just buffer no surface, TODO do we need 
        // make constructor that takes vel and pos
        // TODO CANNOT NEED JELLO< NEED TO COMPUTE ACCELEARTION FOR ALL!!!!!
        //MassPoint* bufferPoint = new MassPoint(*currentPoint->getPosition(), *currentPoint->getVelocity());

        // OH THIS MODEL DOESNT HAVE FIXED< WE CANT JUST SKIP!!!!

        // doubles precision
        

        // first step: k1 = dt * f(x0, t0) -> F1p, F1v
        // * 0.5 for midpoint 
        // Position
        glm::dvec3 dVel = *currentPoint->getVelocity() * cube->timeStep;
        // Velocity
        glm::dvec3 dAcc = *currentPoint->getAcceleration() * cube->timeStep;

        F1p.push_back(dVel);
        F1v.push_back(dAcc);

        glm::dvec3 m_pos = *currentPoint->getPosition() + (dVel * 0.5);
        glm::dvec3 m_vel = *currentPoint->getVelocity() + (dAcc * 0.5);
        
        if (currentPoint->getFixed() == true) {
            // no change
            bufferPoint->setPosition(*currentPoint->getPosition());
            bufferPoint->setVelocity(*currentPoint->getVelocity());
        }
        else {
            bufferPoint->setPosition(m_pos);
            bufferPoint->setVelocity(m_vel);
        }
    }

    computeAcceleration(cube->stiffness, cube->damping, cube->mass, &buffer);

    for (int i = 0; i < cube->discretePoints.size(); i++) {
        MassPoint* currentPoint = cube->discretePoints[i];
        MassPoint* bufferPoint = buffer.discretePoints[i];

        // first step: k1 = dt * f(x0, t0) -> F1p, F1v
        // * 0.5 for midpoint 
        // Position
        glm::dvec3 dVel = *bufferPoint->getVelocity()* cube->timeStep;
        F2p.push_back(dVel);
        
        // Velocity
        glm::dvec3 dAcc = *bufferPoint->getAcceleration() * cube->timeStep;
        F2v.push_back(dAcc);

        glm::dvec3 m_pos = *currentPoint->getPosition() + (dVel * 0.5);
        glm::dvec3 m_vel = *currentPoint->getVelocity() + (dAcc * 0.5);
       
        if (currentPoint->getFixed() == true) {
            // no change
            bufferPoint->setPosition(*currentPoint->getPosition());
            bufferPoint->setVelocity(*currentPoint->getVelocity());
        }
        else {
            bufferPoint->setPosition(m_pos);
            bufferPoint->setVelocity(m_vel);
        }
    }

    computeAcceleration(cube->stiffness, cube->damping, cube->mass, &buffer);

    for (int i = 0; i < cube->discretePoints.size(); i++) {
        MassPoint* currentPoint = cube->discretePoints[i];
        MassPoint* bufferPoint = buffer.discretePoints[i];

        // first step: k1 = dt * f(x0, t0) -> F1p, F1v
        // * 0.5 for midpoint 
        // Position
        glm::dvec3 dVel = *bufferPoint->getVelocity() * cube->timeStep;
        F3p.push_back(dVel);

        // Velocity
        glm::dvec3 dAcc = *bufferPoint->getAcceleration() * cube->timeStep;
        F3v.push_back(dAcc);

        glm::dvec3 m_pos = *currentPoint->getPosition() + dVel;
        glm::dvec3 m_vel = *currentPoint->getVelocity() + dAcc;

        if (currentPoint->getFixed() == true) {
            // no change
            bufferPoint->setPosition(*currentPoint->getPosition());
            bufferPoint->setVelocity(*currentPoint->getVelocity());
        }
        else {
            bufferPoint->setPosition(m_pos);
            bufferPoint->setVelocity(m_vel);
        }
    }

    computeAcceleration(cube->stiffness, cube->damping, cube->mass, &buffer);

    for (int i = 0; i < cube->discretePoints.size(); i++) {
        MassPoint* currentPoint = cube->discretePoints[i];
        MassPoint* bufferPoint = buffer.discretePoints[i];

        // first step: k1 = dt * f(x0, t0) -> F1p, F1v
        // * 0.5 for midpoint 
        // Position
        glm::dvec3 dVel = *bufferPoint->getVelocity() * cube->timeStep;
        F4p.push_back(dVel);

        // Velocity
        glm::dvec3 dAcc = *bufferPoint->getAcceleration() * cube->timeStep;
        F4v.push_back(dAcc);

        if (currentPoint->getFixed() == true) {
            // no change
            continue;
        }

        glm::dvec3 p = *currentPoint->getPosition() + ((F1p[i] + (F2p[i] * 2.0) + (F3p[i] * 2.0) + F4p[i]) / 6.0);
        glm::dvec3 v = *currentPoint->getVelocity() + ((F1v[i] + (F2v[i] * 2.0) + (F3v[i] * 2.0) + F4v[i]) / 6.0);
        currentPoint->setVelocity(v);
        currentPoint->setPosition(p);
        
        //bufferPoint->setPosition(*bufferPoint->getPosition() + *bufferPoint->getVelocity());
        //bufferPoint->setPosition(*bufferPoint->getPosition() + F1v[i]);
        //bufferPoint->setPosition(*bufferPoint->getPosition() + F4p[i]);
        //bufferPoint->setPosition(*bufferPoint->getPosition() / 6.0);
        //currentPoint->setPosition(*bufferPoint->getPosition() + *currentPoint->getPosition());

        //glm::dvec3 v2 = F2v[i] * 2.0;
        //glm::dvec3 v3 = F3v[i] * 2.0;
        //bufferPoint->setPosition(v2);
        //bufferPoint->setVelocity(v3);

        //bufferPoint->setPosition(*bufferPoint->getPosition() + *bufferPoint->getVelocity());
        //bufferPoint->setPosition(*bufferPoint->getPosition() + F1v[i]);
        //bufferPoint->setPosition(*bufferPoint->getPosition() + F4v[i]);
        //bufferPoint->setPosition(*bufferPoint->getPosition() / 6.0);
        //currentPoint->setVelocity(*bufferPoint->getPosition() + *currentPoint->getVelocity());
    }

}