/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The sphere class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Sphere.h"
#include <math.h>

/**
* Sphere's intersection method.  The input is a ray. 
*/
float Sphere::intersect(glm::vec3 p0, glm::vec3 dir) //p0 is the source point and dir is the vector for direction
{
    glm::vec3 vdif = p0 - center;   //Vector s (see Slide 28)  from centre of sphere to the source point
    float b = glm::dot(dir, vdif); // dot product of direction vector and the "s" vector
    float len = glm::length(vdif); // length of the vector from centre to source point
    float c = len*len - radius*radius; // (s . s) - r^2
    float delta = b*b - c; // (s . d)^2 - ((s . s) - r^2) // the stuff insdie the sqrt
   
	if(fabs(delta) < 0.001) return -1.0;  // special cases
    if(delta < 0.0) return -1.0; // special cases

    float t1 = -b - sqrt(delta); // the neg ver of quadratic formula
    float t2 = -b + sqrt(delta); // the pos ver of quadratic formula
    if(fabs(t1) < 0.001 ) // special case
    {
        if (t2 > 0) return t2;
        else t1 = -1.0;
    }
    if(fabs(t2) < 0.001 ) t2 = -1.0; // special case

	return (t1 < t2)? t1: t2; // returns the smallest value. 
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the sphere.
*/
glm::vec3 Sphere::normal(glm::vec3 p)
{
    glm::vec3 n = p - center;
    n = glm::normalize(n);
    return n;
}
