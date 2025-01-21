#pragma once
#include "triangle.h"
#include <vector>
#include <thread>

inline void processVertex(const matrix& p, const matrix& w, const Vertex& mv, const int& width, const int& height, Vertex& out) {
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
	int width = renderer.canvas.getWidth(), height = renderer.canvas.getHeight();

	for (auto& mesh : meshes)
	{
		matrix p = renderer.vp * mesh->world;

		for (int i = 0; i < mesh->triangles.size(); i++)
		{
			Vertex t[3]; // Temporary array to store transformed triangle vertices

			processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[0]], width, height, t[0]);
			processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[1]], width, height, t[1]);
			processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[2]], width, height, t[2]);

			// Clip triangles with Z-values outside [-1, 1]
			if (fabs(t[0].p[2]) > 1.0f || fabs(t[1].p[2]) > 1.0f || fabs(t[2].p[2]) > 1.0f) break;

			// Create a triangle object and render it
			triangle tri(t[0], t[1], t[2]);
			tri.draw(renderer, L, mesh->ka, mesh->kd);
		}
	}
}

