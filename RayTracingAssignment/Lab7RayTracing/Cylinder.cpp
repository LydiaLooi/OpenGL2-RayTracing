/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The cylinder  class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cylinder.h"
#include <math.h>

/**
* Cylinder's intersection method.  The input is a ray.
*/
float Cylinder::intersect(glm::vec3 p0, glm::vec3 dir) //p0 is the source point and dir is the vector for direction
{



    float a = (dir.x * dir.x) + (dir.z * dir.z); // dx^2 + dz^2
    float b = 2 * (dir.x * (p0.x - center.x) + dir.z * (p0.z - center.z)); //(dx(x0 - xc) + dz(z0 - zc)
    float c = (p0.x - center.x) * (p0.x - center.x) + (p0.z - center.z) * (p0.z - center.z) - (radius * radius); // (x0-xc)^2 + (z0 - zc) - r^2


    float delta = b * b - (4 * a * c);


    if (fabs(delta) < 0.001) return -1.0;  // special cases
    if (delta < 0.0) return -1.0; // special cases

    float t1 = (-b - sqrt(delta)) / (2 * a); // the neg ver of quadratic formula
    float t2 = (-b + sqrt(delta)) / (2 * a); // the pos ver of quadratic formula


    float t = -1.0;

   // float t = (t1 < t2) ? t1 : t2;

    if (fabs(t1) < 0.001) // special case
    {
        if (t2 > 0) t = t2;
        else t1 = -1.0;
    }
    if (fabs(t2) < 0.001) t2 = -1.0; // special case

    if (t < 0) {
        t = (t1 < t2) ? t1 : t2;
    }


    float y = p0.y + (dir.y * t);
    if (y >= center.y && y <= (center.y + height)) {
        return t;
    }
    else { // closest point of interesection is outside the cylinder
        t = (t1 < t2) ? t2 : t1;
        y = p0.y + (dir.y * t);
        if (y >= center.y && y <= (center.y + height)) {
            return t;
        }


        return -1.0;
    }

}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the cylinder.
*/
glm::vec3 Cylinder::normal(glm::vec3 p)
{
    glm::vec3 n((p.x - center.x) / radius, 0, (p.z - center.z) / radius);
    return n;

}
