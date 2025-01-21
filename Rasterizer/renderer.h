#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include "GamesEngineeringBase.h"
#include "zbuffer.h"
#include "matrix.h"
#include <mutex>

// The `Renderer` class handles rendering operations, including managing the
// Z-buffer, canvas, and perspective transformations for a 3D scene.
class Renderer {
	float fov = 90.0f * M_PI / 180.0f; // Field of view in radians (converted from degrees)
	float aspect = 4.0f / 3.0f;        // Aspect ratio of the canvas (width/height)
	float n = 0.1f;                    // Near clipping plane distance
	float f = 100.0f;                  // Far clipping plane distance

	matrix perspective;                      // Perspective Projection matrix

	std::mutex zmtx; // lock for z buffer
	std::mutex bmtx; // lock for back buffer

	Zbuffer<float> zbuffer;					 // Z-buffer for depth management
public:
	GamesEngineeringBase::Window canvas;     // Canvas for rendering the scene
	matrix vp;                               // view projection matrix

	// Constructor initializes the canvas, Z-buffer, and perspective projection matrix.
	Renderer() {
		canvas.create(1024, 768, "Raster");  // Create a canvas with specified dimensions and title
		zbuffer.create(1024, 768);           // Initialize the Z-buffer with the same dimensions
		perspective = matrix::makePerspective(fov, aspect, n, f); // Set up the perspective matrix
	}

	// Clears the canvas and resets the Z-buffer.
	void clear() {
		canvas.clear();  // Clear the canvas (sets all pixels to the background color)
		zbuffer.clear(); // Reset the Z-buffer to the farthest depth
	}

	// Presents the current canvas frame to the display.
	void present() {
		canvas.present(); // Display the rendered frame
	}

	// update view projection matrix
	void updateVP(const matrix& view) {
		vp = perspective * view;
	}

	void draw(const unsigned int& index, unsigned char* _color) {
		//std::lock_guard<std::mutex> lock(bmtx);
		canvas.draw(index, _color);
	}

	void setDepth(const unsigned int& index, float val) {
		//std::lock_guard<std::mutex> lock(zmtx);
		zbuffer[index] = val;
	}

	float getDepth(const unsigned int& index) {
		//std::lock_guard<std::mutex> lock(zmtx);
		return zbuffer[index];
	}
};