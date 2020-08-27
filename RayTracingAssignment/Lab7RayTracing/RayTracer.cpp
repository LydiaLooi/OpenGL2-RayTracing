/*==================================================================================
* COSC 363  Computer Graphics (2020)
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab07.pdf, Lab08.pdf for details.
*===================================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Ray.h"
#include <GL/freeglut.h>
#include "Plane.h"
#include "TextureBMP.h"
#include "Cylinder.h"

using namespace std;

const float WIDTH = 20.0;  
const float HEIGHT = 20.0;
const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -WIDTH * 0.5;
const float XMAX =  WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX =  HEIGHT * 0.5;

glm::vec3 TEXT_SPHERE_CENTRE(-7.0, 6.0, -88.0);
glm::vec3 TEXT_SPHERE_CENTRE2(4.0, 2.0, -65.0);
glm::vec3 SPOT_SPHERE_CENTRE(-6.5, -3.5, -40.);
float PI = 3.14159265359;




vector<SceneObject*> sceneObjects;

TextureBMP texture;
TextureBMP texture2;

glm::vec3 average2Colours(glm::vec3 colour1, glm::vec3 colour2) {
	float r = sqrt(((colour1.r * colour1.r) + (colour2.r * colour2.r)) / 2);
	float g = sqrt(((colour1.g * colour1.g) + (colour2.g * colour2.g)) / 2);
	float b = sqrt(((colour1.b * colour1.b) + (colour2.b * colour2.b)) / 2);
	glm::vec3 averageColour(r, g, b);
	return averageColour;
}


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(1.0);						//Background colour = (0,0,0)
	glm::vec3 lightPos(30, 40, -3);					//Light's position

	glm::vec3 spotlightPos(-14., 20., -10.); ///spotlight poisition
	float spotAngle = 0.122173; //7degrees
	glm::vec3 spotlightDir(SPOT_SPHERE_CENTRE - spotlightPos);

	glm::vec3 color(0);
	SceneObject* obj;

    ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
    if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found



	color = obj->lighting(lightPos, -ray.dir, ray.hit);




	// ===== CHECKERED =========//
	if (obj->isCheckered()) 
	{
		//Checkered pattern
		int stripeWidth = 5;
		int iz = (ray.hit.z) / stripeWidth;
		int ix = (ray.hit.x) / stripeWidth;
		
		int k = (iz + ix) % 2; //2 colors

		if (ray.hit.x >= 0) {
			if (k == 0) {
				k = 1;
			}
			else {
				k = 0;
			}
		}

	
		if (k == 0) {
			color = glm::vec3(1, 0.5, 0);
		}
		else {
			color = glm::vec3(0.2, 0.2, 0.2);
		}
		obj->setColor(color);
	}



	

	



	


	// ====== CASTED SHADOWS =========== //

	glm::vec3 lightVec = lightPos - ray.hit;
	Ray shadowRay(ray.hit, lightVec);

	shadowRay.closestPt(sceneObjects);

	float lightDist = glm::length(lightVec);


	if (shadowRay.index > -1) {
		float ambient = 0.2;

		SceneObject* hitObject;
		hitObject = sceneObjects[shadowRay.index];
		
		if (shadowRay.dist < lightDist) {
			if (hitObject->isTransparent() || hitObject->isRefractive()) {
				ambient = 0.5;
			}
			if (obj->isTransparent()) {
				color = ambient * color;
			}
			else {
				color = ambient * obj->getColor();
			}

			if (hitObject->isTransparent() && obj != hitObject) {
				color = color * (hitObject->getColor() + 1.0f);
			}
			
		}

	}



	// SPOT LIGHT CHECK //

	glm::vec3 spotlightVec = spotlightPos - ray.hit;
	Ray shadowRay2(ray.hit, spotlightVec);
	shadowRay2.closestPt(sceneObjects);

	float spotLightDist = glm::length(spotlightVec);
float ambient2 = 0.5;
if (shadowRay2.index > -1) {
	SceneObject* hitObject2;
	hitObject2 = sceneObjects[shadowRay2.index];

	if (shadowRay2.dist < spotLightDist && hitObject2->isSpotlit()) { // only cast a shadow if its for the spotlit object
		if (hitObject2->isTransparent() || hitObject2->isRefractive()) {
			ambient2 = 0.9;
		}
		color = ambient2 * color;
	}
}


else { // not in shadow!

	glm::vec3 u = -spotlightVec;
	glm::vec3 v = spotlightDir;

	float udotv2 = (u.x * v.x) + (u.y * v.y) + (u.z * v.z);



	float udotv = glm::dot(u, v);
	float uMag = sqrt((u.x * u.x) + (u.y * u.y) + (u.z * u.z));
	float vMag = sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	float angle = acos(udotv / (uMag * vMag));


	if (angle < spotAngle) {
		color = average2Colours(color, color + 0.2f);
	}

}


// ============ TRANSPARENCY =============== //
if (obj->isTransparent()) {

	Ray piercingRay(ray.hit, ray.dir);
	glm::vec3 resultCol = trace(piercingRay, step + 1);

	color = average2Colours(obj->getColor(), color);

	color = average2Colours(color, resultCol);


}


if (obj->isTextured()) { // https://en.wikipedia.org/wiki/UV_mapping
	glm::vec3 d;

	d = glm::normalize(ray.hit - TEXT_SPHERE_CENTRE2);
	float texcoords = 0.5 + (atan2(d.x, d.z) / (2 * PI));
	float texcoordt = 0.5 + (asin(d.y) / (PI));
	color = average2Colours(color, texture2.getColorAt(texcoords, texcoordt));

	glm::vec3 result = texture.getColorAt(texcoords, texcoordt);

	float minThreshold = 0.2;
	if (result.r > minThreshold && result.g > minThreshold && result.b > minThreshold) {
		color = color + result; // clouds
	}

}



// =========== REFLECTIVE ================== //
if (obj->isReflective() && step < MAX_STEPS) {
	float rho = obj->getReflectionCoeff();
	glm::vec3 normalVec = obj->normal(ray.hit);
	glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
	Ray reflectedRay(ray.hit, reflectedDir);

	if (normalVec.z > 0) {

		glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
		color = color + (rho * reflectedColor);
	}

}



// ======== REFRACTIVE ============= //

if (obj->isRefractive()) {
	glm::vec3 n = obj->normal(ray.hit);

	float eta = 1 / obj->getRefractiveIndex();

	glm::vec3 g = glm::refract(ray.dir, n, eta);
	Ray refrRay(ray.hit, g);
	refrRay.closestPt(sceneObjects);
	glm::vec3 m = obj->normal(refrRay.hit);
	glm::vec3 h = glm::refract(g, -m, 1.0f / eta);

	Ray resultRay(refrRay.hit, h);
	color = color + trace(resultRay, step + 1);
}


// =========== FOG ==================== //

float z1 = -60;
float z2 = -200;

glm::vec3 white(1., 1., 1.);

glm::vec3 black(0., 0., 0.);

if (ray.hit.z <= z1 && ray.hit.z >= z2) {
	float t = (ray.hit.z - z1) / (z2 - z1);
	color = (1.0f - t) * color + t * white;
}




	return color;
}




//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float xp, yp;  //grid point
	float cellX = (XMAX-XMIN)/NUMDIV;  //cell width
	float cellY = (YMAX-YMIN)/NUMDIV;  //cell height
	glm::vec3 eye(0., -1.0, 0.);

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a tiny quad.

	for(int i = 0; i < NUMDIV; i++)	//Scan every cell of the image plane
	{
		xp = XMIN + i*cellX;
		for(int j = 0; j < NUMDIV; j++) {


			yp = YMIN + j*cellY;

			bool aa = true; // true = enable anti aliasing

			glm::vec3 col;

			if (!aa) {
				glm::vec3 dir1(xp + 0.5 * cellX, yp + 0.5 * cellY, -EDIST);	//direction of the primary ray
				Ray ray1 = Ray(eye, dir1);
				col = trace(ray1, 1); //Trace

			}
			else {
				
				glm::vec3 dir1(xp + 0.25 * cellX, yp + 0.25 * cellY, -EDIST);
				glm::vec3 dir2(xp + 0.25 * cellX, yp - 0.25 * cellY, -EDIST);
				glm::vec3 dir3(xp - 0.25 * cellX, yp + 0.25 * cellY, -EDIST);
				glm::vec3 dir4(xp - 0.25 * cellX, yp - 0.25 * cellY, -EDIST);

				Ray ray1 = Ray(eye, dir1);
				Ray ray2 = Ray(eye, dir2);
				Ray ray3 = Ray(eye, dir3);
				Ray ray4 = Ray(eye, dir4);

				
				glm::vec3 col1 = trace(ray1, 1); //Trace
				glm::vec3 col2 = trace(ray2, 1); //Trace
				glm::vec3 col3 = trace(ray3, 1); //Trace
				glm::vec3 col4 = trace(ray4, 1); //Trace


				col.r = (col1.r  + col2.r + col3.r + col4.r) / 4.0f;
				col.g = (col1.g  + col2.g  + col3.g  + col4.g ) / 4.0f;
				col.b = (col1.b  + col2.b + col3.b + col4.b ) / 4.0f;

			}





			glColor3f(col.r, col.g, col.b);

			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp+cellX, yp);
			glVertex2f(xp+cellX, yp+cellY);
			glVertex2f(xp, yp+cellY);
        }
    }

    glEnd();
    glFlush();
}



//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

	texture = TextureBMP("clouds.bmp");
	texture2 = TextureBMP("earth.bmp");

    glClearColor(0, 0, 0, 1);

	// ============= Objects ===============


	// ===== Checkered Floor plane ===== //
	Plane* floor = new Plane(
		glm::vec3(-500., -10, 0),
		glm::vec3(500., -10, 0),
		glm::vec3(500., -10, -200),
		glm::vec3(-500., -10, -200)
		);
	floor->setColor(glm::vec3(1, 0.5, 0));
	floor->setCheckered(true);
	floor->setSpecularity(false);
	sceneObjects.push_back(floor);

	// =============================== //


	
	Plane* tableTop = new Plane(
		glm::vec3(5.0, -4, -30),
		glm::vec3(19., -4, -30),
		glm::vec3(3.0, -4, -100),
		glm::vec3(-21., -4, -100)
		);
	tableTop->setColor(glm::vec3(0.5, 0.5, 0.5));
	tableTop->setSpecularity(false);
	tableTop->setReflectivity(true, 0.3);

	sceneObjects.push_back(tableTop);

	Plane* tableBottom = new Plane(
		glm::vec3(5., -5.5, -30),
		glm::vec3(19., -5.5, -30),
		glm::vec3(3., -5.5, -100),
		glm::vec3(-21., -5.5, -100)
		);
	tableBottom->setColor(glm::vec3(0.5, 0.5, 0.5));
	tableBottom->setSpecularity(false);

	sceneObjects.push_back(tableBottom);

	Plane* tableFront = new Plane(
		glm::vec3(5., -5.5, -30),
		glm::vec3(19., -5.5, -30),
		glm::vec3(19., -4., -30),
		glm::vec3(5., -4., -30)
		);
	tableFront->setColor(glm::vec3(0.5, 0.5, 0.5));
	tableFront->setSpecularity(false);
	tableFront->setReflectivity(true, 0.1);

	sceneObjects.push_back(tableFront);

	Plane* tableSide = new Plane(
		glm::vec3(-21., -5.5, -100),
		glm::vec3(5., -5.5, -30),
		glm::vec3(5., -4., -30),
		glm::vec3(-21., -4., -100)
		);
	tableSide->setColor(glm::vec3(0.5, 0.5, 0.5));
	tableSide->setSpecularity(false);
	tableSide->setReflectivity(true, 0.1);

	sceneObjects.push_back(tableSide);

	


	// ================================ //

	
	

	Cylinder* tableLeg1 = new Cylinder(glm::vec3(7.0, -10.0, -33.0), 1.0, 4.5);
	tableLeg1->setColor(glm::vec3(0.5, 0.1, 0));   
	tableLeg1->setReflectivity(true, 0.05);
	sceneObjects.push_back(tableLeg1);


	Cylinder* tableLeg3 = new Cylinder(glm::vec3(-16, -10.0, -98.0), 1.0, 4.5);
	tableLeg3->setColor(glm::vec3(0.5, 0.1, 0));
	tableLeg3->setReflectivity(true, 0.05);
	sceneObjects.push_back(tableLeg3);



	Sphere *bigBoiSphere = new Sphere(TEXT_SPHERE_CENTRE, 10.0);
	bigBoiSphere->setColor(glm::vec3(1, 0.5, 0));
	//bigBoiSphere->setTextured(true, 0);

	bigBoiSphere->setReflectivity(true, 0.4);

	sceneObjects.push_back(bigBoiSphere);
	
	
	Sphere* sphere2 = new Sphere(TEXT_SPHERE_CENTRE2, 6.0);
	sphere2->setColor(glm::vec3(0.2, 0.2, 0.5));
	sphere2->setTextured(true, 1);
	sphere2->setReflectivity(true, 0.1);
	sphere2->setShininess(6);
	sceneObjects.push_back(sphere2);		 //Add sphere to scene objects
	
	Cylinder* cylinder = new Cylinder(glm::vec3(-6.5, -10.0, -40.), .5, 3.5); //-6.5, -5., -45.
	cylinder->setColor(glm::vec3(0.3, 0.3, 0.9));
	cylinder->setReflectivity(true, 0.1);
	cylinder->setSpotlit(true);
	sceneObjects.push_back(cylinder);

	Cylinder* cylinder2 = new Cylinder(glm::vec3(-6.5, -10.0, -40.), 2., 1.5); //-6.5, -5., -45.
	cylinder2->setColor(glm::vec3(0.9, 0.3, 0.3));
	cylinder2->setReflectivity(true, 0.1);
	cylinder2->setSpotlit(true);
	sceneObjects.push_back(cylinder2);

	Cylinder* cylinder3 = new Cylinder(glm::vec3(-6.5, -10.0, -40.), 1., 2.5); //-6.5, -5., -45.
	cylinder3->setColor(glm::vec3(0.3, 0.9, 0.3));
	cylinder3->setReflectivity(true, 0.1);
	cylinder3->setSpotlit(true);
	sceneObjects.push_back(cylinder3);
	
	Sphere* sphere3 = new Sphere(SPOT_SPHERE_CENTRE, 3.0); 
	sphere3->setColor(glm::vec3(0, 0, 0));   
	sphere3->setRefractivity(true, 1.0f, 1.5f);
	sphere3->setReflectivity(true, 0.4);
	sphere3->setSpotlit(true);

	sceneObjects.push_back(sphere3);		 //Add sphere to scene objects
	

	Sphere* sphere4 = new Sphere(glm::vec3(5., -1., -45.0), 3.0);
	sphere4->setColor(glm::vec3(0.5, 0., 0.));  
	sphere4->setTransparency(true);
	sphere4->setReflectivity(true, 0.1);
	sceneObjects.push_back(sphere4);		 //Add sphere to scene objects

	Sphere* sphere5 = new Sphere(glm::vec3(6.5, -2., -35.0), 2.0);
	sphere5->setColor(glm::vec3(0., 0., 0.7));
	sphere5->setTransparency(true);
	sphere5->setReflectivity(true, 0.1);
	sceneObjects.push_back(sphere5);		 //Add sphere to scene objects

}


int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracing");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
