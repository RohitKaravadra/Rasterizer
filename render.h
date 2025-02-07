#pragma once
#include "triangle.h"
#include <vector>
#include <thread>
#include "sentinelQueue.h"

// store temporary data for triangle rendering
struct triangleData
{
	triangle tri;	// triangle
	color a;		// ambient light
	color d;		// diffuse light

	triangleData() = default;
	triangleData(triangle _tri, color _a, color _d) :tri(_tri), a(_a), d(_d) {
	}
};

static class Render
{
	static std::atomic<int> triCounter;		// atomic triangle index counter for threads
	static std::atomic<int> meshCounter;	// atomic mesh index counter for threads
	static std::atomic<bool> meshProcessed;	// indicator for triangle threads to join

	static SentinelQueue<triangleData> queue;

	// process vertex for triangle
	// Input Variables:
	// - p : projection matrix
	// - w : world matrix of mesh
	// - mv	: mesh vertex
	// - width : width of canvas
	// - height : height of canvas
	static inline void processVertex(const matrix& p, const matrix& w, const Vertex& mv,
		const unsigned int& width, const unsigned int& height, Vertex& out);

	// Method to draw triangles with multi threading
	// Input Variables:
	// - tris		: pointer to triangle array 
	// - total		: size of triangle array 
	// - renderer	: reference to renderer 
	// - L			: reference to Light
	static void drawTriangles(triangleData* tris, int total, Renderer& renderer, vec4 lightDir);

public:
	// method processes and draws triangles using caching
	// - meshes	: array of meshes
	// - renderer : reference to the renderer
	// - L : light
	// default value set to 3 works best for this value
	static void renderCaching(const std::vector<Mesh*>& meshes, Renderer& renderer, Light& L);

	// method processes and draws triangles using shared counter
	// - meshes	: array of meshes
	// - renderer : reference to the renderer
	// - L : light
	// - totalThreads : number of threads to use for multithreading
	// default value set to 3 works best for this value
	static void renderSharedCounter(const std::vector<Mesh*>& meshes, Renderer& renderer, Light& L, unsigned int totalThreads = 3);


	static void processMesh(const std::vector<Mesh*>& meshes, int total,
		const unsigned int& width, const unsigned int& height, matrix vp, Light L);

	static void processTriangles(Renderer& renderer, const vec4& dir);

	// method processes and draws triangles using shared counter
	// - meshes	: array of meshes
	// - renderer : reference to the renderer
	// - L : light
	// - totalThreads : number of threads to use for multithreading
	// default value set to 3 works best for this value
	static void renderSentinelQueue(const std::vector<Mesh*>& meshes, Renderer& renderer, Light& L,
		unsigned int meshThreadCount = 3, unsigned int triThreadCount = 3);
};

