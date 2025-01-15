#pragma once
#include "renderer.h"
#include "mesh.h"
#include "light.h"

inline void ProcessVertex(const matrix& p, const matrix& w, const Vertex& mv, const float& width, const float& height, Vertex& out)
{
	out.p = p * mv.p; // Apply transformations
	out.p.divideW(); // Perspective division to normalize coordinates

	// Transform normals into world space for accurate lighting
	// no need for perspective correction as no shearing or non-uniform scaling
	out.normal = w * mv.normal;
	out.normal.normalise();

	// Map normalized device coordinates to screen space
	out.p[0] = (out.p[0] + 1.f) * 0.5f * width;
	out.p[1] = (out.p[1] + 1.f) * 0.5f * height;
	out.p[1] = height - out.p[1]; // Invert y-axis

	// Copy vertex colours
	out.rgb = mv.rgb;
}

inline void ProcessTriangle(const Mesh& mesh, int i, const matrix& p, Renderer& renderer, Light& L, int width, int height)
{
	Vertex t[3]; // Temporary array to store transformed triangle vertices

	ProcessVertex(p, mesh.world, mesh.vertices[mesh.triangles[i].v[0]], width, height, t[0]);
	ProcessVertex(p, mesh.world, mesh.vertices[mesh.triangles[i].v[1]], width, height, t[1]);
	ProcessVertex(p, mesh.world, mesh.vertices[mesh.triangles[i].v[2]], width, height, t[2]);

	// Clip triangles with Z-values outside [-1, 1]
	if (fabs(t[0].p[2]) > 1.0f || fabs(t[1].p[2]) > 1.0f || fabs(t[2].p[2]) > 1.0f) return;

	// Create a triangle object and render it
	triangle tri(t[0], t[1], t[2]);
	tri.draw(renderer, L, mesh.ka, mesh.kd);
}

// Main rendering function that processes a mesh, transforms its vertices, applies lighting, and draws triangles on the canvas.
// Input Variables:
// - renderer: The Renderer object used for drawing.
// - mesh: Pointer to the Mesh object containing vertices and triangles to render.
// - camera: Matrix representing the camera's transformation.
// - L: Light object representing the lighting parameters.
static void render(Renderer& renderer, Mesh* mesh, Light& L) {
	// Combine perspective, camera, and world transformations for the mesh
	matrix p = renderer.vp * mesh->world;

	// Iterate through all triangles in the mesh
	//int i = 0;
	//for (; i < mesh->triangles.size(); i++) {
	//	ProcessTriangle(*mesh, i, p, renderer, L, canWidth, canHeight);
	//}

	// Iterate through all triangles in the mesh
	for (triIndices& ind : mesh->triangles) {
		Vertex t[3]; // Temporary array to store transformed triangle vertices
	
		// Transform each vertex of the triangle
		for (unsigned int i = 0; i < 3; i++) {
			t[i].p = p * mesh->vertices[ind.v[i]].p; // Apply transformations
			t[i].p.divideW(); // Perspective division to normalize coordinates
	
			// Transform normals into world space for accurate lighting
			// no need for perspective correction as no shearing or non-uniform scaling
			t[i].normal = mesh->world * mesh->vertices[ind.v[i]].normal;
			t[i].normal.normalise();
	
			// Map normalized device coordinates to screen space
			t[i].p[0] = (t[i].p[0] + 1.f) * 0.5f * renderer.canvas.getWidth();
			t[i].p[1] = (t[i].p[1] + 1.f) * 0.5f * renderer.canvas.getHeight();
			t[i].p[1] = renderer.canvas.getHeight() - t[i].p[1]; // Invert y-axis
	
			// Copy vertex colours
			t[i].rgb = mesh->vertices[ind.v[i]].rgb;
		}
	
		// Clip triangles with Z-values outside [-1, 1]
		if (fabs(t[0].p[2]) > 1.0f || fabs(t[1].p[2]) > 1.0f || fabs(t[2].p[2]) > 1.0f) continue;
	
		// Create a triangle object and render it
		triangle tri(t[0], t[1], t[2]);
		tri.draw(renderer, L, mesh->ka, mesh->kd);
	}
}

// Utility function to generate a random rotation matrix
// No input variables
static matrix makeRandomRotation() {
	RandomNumberGenerator& rng = RandomNumberGenerator::getInstance();
	unsigned int r = rng.getRandomInt(0, 3);

	switch (r) {
	case 0: return matrix::makeRotateX(rng.getRandomFloat(0.f, 2.0f * M_PI));
	case 1: return matrix::makeRotateY(rng.getRandomFloat(0.f, 2.0f * M_PI));
	case 2: return matrix::makeRotateZ(rng.getRandomFloat(0.f, 2.0f * M_PI));
	default: return matrix::makeIdentity();
	}
}


