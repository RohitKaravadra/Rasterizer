#pragma once
#include "triangle.h"
#include <vector>
#include <thread>

// store temporary data for triangle rendering
struct triangleData
{
	triangle tri;	// triangle
	color a;		// ambient light
	color d;		// diffuse light

	triangleData(triangle _tri, color _a, color _d) :tri(_tri), a(_a), d(_d) {
	}
};

// process vertex for triangle
// Input Variables:
// - p: projection matrix
// - w: world matrix of mesh
// - mv: mesh vertex
// - width: width of canvas
// - height: height of canvas
inline void processVertex(const matrix& p, const matrix& w, const Vertex& mv,
	const unsigned int& width, const unsigned int& height, Vertex& out)
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

static void render(const std::vector<Mesh*>& meshes, Renderer& renderer, Light& L) {

	L.omega_i.normalise(); // normalize light before rendering

	// cache canvas width and height
	unsigned int width = renderer.canvas.getWidth();
	unsigned int height = renderer.canvas.getHeight();

	for (auto& mesh : meshes)
	{
		matrix p = renderer.vp * mesh->world; // calculate projection matrix for the mesh

		// calculate diffuse and ambient lights for mesh
		color ambient = L.ambient * mesh->ka;
		color diffuse = L.L * mesh->kd;

		// process all triangles of mesh
		for (int i = 0; i < mesh->triangles.size(); i++)
		{
			Vertex t[3]; // Temporary array to store transformed triangle vertices

			// process all 3 vertices of triangles
			processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[0]], width, height, t[0]);
			processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[1]], width, height, t[1]);
			processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[2]], width, height, t[2]);

			// Clip triangles with Z-values outside [-1, 1]
			if (fabs(t[0].p[2]) > 1.0f || fabs(t[1].p[2]) > 1.0f || fabs(t[2].p[2]) > 1.0f) break;

			// Create and render triangle object 
			triangle(t[0], t[1], t[2]).draw(renderer, L.omega_i, ambient, diffuse);
		}
	}
}

static std::atomic<int> triCounter; // triangle index atomic counter for threads

// Method to draw triangles with multi threading
// Input Variables:
// - tris: pointer to triangle array 
// - total: size of triangle array 
// - renderer: reference to renderer 
// - L: reference to Light
static void drawTriangles(triangleData* tris, int total, Renderer& renderer, Light& L)
{
	int i;
	while ((i = triCounter.fetch_add(1)) < total)
		tris[i].tri.draw(renderer, L.omega_i, tris[i].a, tris[i].d);
}

static void renderMT(const std::vector<Mesh*>& meshes, Renderer& renderer, Light& L)
{
	L.omega_i.normalise(); // normalize light before rendering

	// cache canvas width and height
	unsigned int width = renderer.canvas.getWidth();
	unsigned int height = renderer.canvas.getHeight();

	std::vector<triangleData> triangles;

	for (auto& mesh : meshes)
	{
		matrix p = renderer.vp * mesh->world; // calculate projection matrix for the mesh

		// calculate diffuse and ambient lights for mesh
		color ambient = L.ambient * mesh->ka;
		color diffuse = L.L * mesh->kd;

		// process all triangles of mesh
		for (int i = 0; i < mesh->triangles.size(); i++)
		{
			Vertex t[3]; // Temporary array to store transformed triangle vertices

			// process all 3 vertices of triangles
			processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[0]], width, height, t[0]);
			processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[1]], width, height, t[1]);
			processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[2]], width, height, t[2]);

			// Clip triangles with Z-values outside [-1, 1]
			if (fabs(t[0].p[2]) > 1.0f || fabs(t[1].p[2]) > 1.0f || fabs(t[2].p[2]) > 1.0f) break;

			// add triangle to triangle list
			triangles.emplace_back(triangleData(triangle(t[0], t[1], t[2]), ambient, diffuse));
		}
	}

	unsigned int totalThreads = 3; // number of threads to use
	unsigned int size = triangles.size(); // total triangles count

	triCounter.store(0); // reset triangle counter

	// render triangle using multiple threads
	std::vector<std::thread> threads; // threads array
	for (int i = 0; i < totalThreads; i++)
		threads.emplace_back(std::thread(drawTriangles, &triangles[0], size, std::ref(renderer), std::ref(L)));

	for (auto& t : threads)
		t.join();
}

