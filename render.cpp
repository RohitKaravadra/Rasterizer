#pragma once
#include "render.h"


std::atomic<int> Render::triCounter;
std::atomic<int> Render::meshCounter;

SentinelQueue<triangleData> Render::queue;
std::atomic<bool> Render::meshProcessed;

void Render::processVertex(const matrix& p, const matrix& w, const Vertex& mv,
	const unsigned int& width, const unsigned int& height, Vertex& out)
{
	out.p = p * mv.p;					// Apply transformations
	out.p.divideW();					// Perspective division to normalize coordinates

	// Transform normals into world space for accurate lighting
	// no need for perspective correction as no shearing or non-uniform scaling
	out.normal = w * mv.normal;
	out.normal.normalise();

	// Map normalized device coordinates to screen space
	out.p[0] = (out.p[0] + 1.f) * 0.5f * width;
	out.p[1] = (out.p[1] + 1.f) * 0.5f * height;
	out.p[1] = height - out.p[1];		// Invert y-axis

	// Copy vertex colours
	out.rgb = mv.rgb;
}

void Render::drawTriangles(triangleData* tris, int total, Renderer& renderer, vec4 lightDir)
{
	int i;
	while ((i = triCounter.fetch_add(1)) < total)
		tris[i].tri.drawIncremental(renderer, lightDir, tris[i].a, tris[i].d);
}

void Render::renderCaching(const std::vector<Mesh*>& meshes, Renderer& renderer, Light& L) {

	L.omega_i.normalise(); // normalize light before rendering

	// cache canvas width and height
	unsigned int width = renderer.canvas.getWidth();
	unsigned int height = renderer.canvas.getHeight();

	for (auto& mesh : meshes)
	{
		matrix p = renderer.vp * mesh->world;	// calculate projection matrix for the mesh

		// calculate diffuse and ambient lights for mesh
		color ambient = L.ambient * mesh->ka;
		color diffuse = L.L * mesh->kd;

		// process all triangles of mesh
		for (int i = 0; i < mesh->triangles.size(); i++)
		{
			Vertex t[3];						// Temporary array to store transformed triangle vertices

			// process all 3 vertices of triangles (loop unrolling)
			processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[0]], width, height, t[0]);
			processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[1]], width, height, t[1]);
			processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[2]], width, height, t[2]);

			// Clip triangles with Z-values outside [-1, 1]
			if (fabs(t[0].p[2]) > 1.0f || fabs(t[1].p[2]) > 1.0f || fabs(t[2].p[2]) > 1.0f) break;

			// Create and render triangle object 
			triangle(t[0], t[1], t[2]).drawIncremental(renderer, L.omega_i, ambient, diffuse);
		}
	}
}

void Render::renderSharedCounter(const std::vector<Mesh*>& meshes, Renderer& renderer, Light& L, unsigned int totalThreads)
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

			// process all 3 vertices of triangles (loop unrolling)
			processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[0]], width, height, t[0]);
			processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[1]], width, height, t[1]);
			processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[2]], width, height, t[2]);

			// Clip triangles with Z-values outside [-1, 1]
			if (fabs(t[0].p[2]) > 1.0f || fabs(t[1].p[2]) > 1.0f || fabs(t[2].p[2]) > 1.0f) break;

			// add triangle to triangle list
			triangles.emplace_back(triangleData(triangle(t[0], t[1], t[2]), ambient, diffuse));
		}
	}

	unsigned int size = triangles.size(); // total triangles count

	triCounter.store(0); // reset triangle counter

	// render triangle using multiple threads
	std::vector<std::thread> threads; // threads array
	for (int i = 0; i < totalThreads; i++)
		threads.emplace_back(std::thread(drawTriangles, &triangles[0], size, std::ref(renderer), L.omega_i));

	for (auto& t : threads)
		t.join();
}

void Render::processMesh(const std::vector<Mesh*>& meshes, int total,
	const unsigned int& width, const unsigned int& height, matrix vp, Light L)
{
	int i;
	while ((i = meshCounter.fetch_add(1)) < total)
	{
		Mesh* mesh = meshes[i];
		matrix p = vp * mesh->world; // calculate projection matrix for the mesh

		// calculate diffuse and ambient lights for mesh
		color ambient = L.ambient * mesh->ka;
		color diffuse = L.L * mesh->kd;

		// process all triangles of mesh
		for (int i = 0; i < mesh->triangles.size(); i++)
		{
			Vertex t[3]; // Temporary array to store transformed triangle vertices

			// process all 3 vertices of triangles (loop unrolling)
			processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[0]], width, height, t[0]);
			processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[1]], width, height, t[1]);
			processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[2]], width, height, t[2]);

			// Clip triangles with Z-values outside [-1, 1]
			if (fabs(t[0].p[2]) > 1.0f || fabs(t[1].p[2]) > 1.0f || fabs(t[2].p[2]) > 1.0f) break;

			// add triangle to triangle list
			queue.enqueue(triangleData(triangle(t[0], t[1], t[2]), ambient, diffuse));
		}
	}
}

void Render::processTriangles(Renderer& renderer, const vec4& dir)
{
	triangleData data;
	bool process;
	while ((process = queue.dequeue(data)) || !meshProcessed)
	{
		if (process)
			data.tri.drawIncremental(renderer, dir, data.a, data.d);
	}
}

void Render::renderSentinelQueue(const std::vector<Mesh*>& meshes, Renderer& renderer, Light& L, unsigned int meshThreadCount, unsigned int triThreadCount)
{
	L.omega_i.normalise(); // normalize light before rendering

	// cache canvas width and height
	unsigned int width = renderer.canvas.getWidth();
	unsigned int height = renderer.canvas.getHeight();

	triCounter.store(0);
	meshCounter.store(0);
	meshProcessed.store(0);

	std::vector<std::thread> meshThreads;	// mesh threads array
	std::vector<std::thread> triThreads;	// triangles threads array

	for (int i = 0; i < meshThreadCount; i++)
		meshThreads.emplace_back(std::thread(processMesh, std::ref(meshes), meshes.size(), width, height, renderer.vp, L));

	for (int i = 0; i < triThreadCount; i++)
		triThreads.emplace_back(std::thread(processTriangles, std::ref(renderer), L.omega_i));

	for (auto& t : meshThreads)
		t.join();

	meshProcessed.store(1);

	for (auto& t : triThreads)
		t.join();
}

