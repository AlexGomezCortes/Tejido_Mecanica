#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include "OurHeader.h"

// Boolean variables allow to show/hide the primitives



void setupPrims() {
	Sphere::setupSphere();
	Capsule::setupCapsule();
	LilSpheres::setupParticles(LilSpheres::maxParticles);
	ClothMesh::setupClothMesh();
	Cube::setupCube();
}
void cleanupPrims() {
	Sphere::cleanupSphere();
	Capsule::cleanupCapsule();
	LilSpheres::cleanupParticles();
	ClothMesh::cleanupClothMesh();
	Cube::cleanupCube();
}

void renderPrims() {
	if (renderSphere)
		Sphere::drawSphere();
	if (renderCapsule)
		Capsule::drawCapsule();

	if (renderParticles) {
		// You may need to rethink this piece of code...
		int startDrawingFromParticle = 0;
		int numParticlesToDraw = LilSpheres::maxParticles;
		LilSpheres::drawParticles(startDrawingFromParticle, numParticlesToDraw);
		// .............................................
	}
	
	if (renderCloth)
		ClothMesh::drawClothMesh();

	if (renderCube)
		Cube::drawCube();
}
