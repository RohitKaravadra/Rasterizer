#include "Includes.h"

// debug timer
static ChronoClock<std::chrono::high_resolution_clock> timer;

// Test scene function to demonstrate rendering with user-controlled transformations
// No input variables
void sceneTest() {
	Renderer renderer;
	// create light source {direction, diffuse intensity, ambient intensity}
	Light L{ vec4(0.f, 1.f, 1.f, 0.f), colour(1.0f, 1.0f, 1.0f), colour(0.1f, 0.1f, 0.1f) };
	// camera is just a matrix
	matrix camera = matrix::makeIdentity(); // Initialize the camera with identity matrix

	bool running = true; // Main loop control variable

	std::vector<Mesh*> scene; // Vector to store scene objects

	// Create a sphere and a rectangle mesh
	Mesh mesh = Mesh::makeSphere(1.0f, 10, 20);
	//Mesh mesh2 = Mesh::makeRectangle(-2, -1, 2, 1);

	// add meshes to scene
	scene.push_back(&mesh);
	//scene.push_back(&mesh2);

	float x = 0.0f, y = 0.0f, z = -4.0f; // Initial translation parameters
	mesh.world = matrix::makeTranslation(x, y, z);
	//mesh2.world = matrix::makeTranslation(x, y, z) * matrix::makeRotateX(0.01f);

	// Main rendering loop
	while (running) {
		renderer.canvas.checkInput(); // Handle user input
		renderer.clear(); // Clear the canvas for the next frame

		timer.enable = renderer.canvas.keyPressed(VK_SPACE);
		timer.Start();
		// Apply transformations to the meshes
		//mesh2.world = matrix::makeTranslation(x, y, z) *matrix::makeRotateX(0.01f);
		mesh.world = matrix::makeTranslation(x, y, z);

		// Handle user inputs for transformations
		if (renderer.canvas.keyPressed(VK_ESCAPE)) break;
		if (renderer.canvas.keyPressed('A')) x += -0.1f;
		if (renderer.canvas.keyPressed('D')) x += 0.1f;
		if (renderer.canvas.keyPressed('W')) y += 0.1f;
		if (renderer.canvas.keyPressed('S')) y += -0.1f;
		if (renderer.canvas.keyPressed('Q')) z += 0.1f;
		if (renderer.canvas.keyPressed('E')) z += -0.1f;

		// Render each object in the scene
		for (auto& m : scene)
			render(renderer, m, camera, L);

		renderer.present(); // Display the rendered frame
		timer.Stop();
		timer.Print();
	}
}