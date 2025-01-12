#include "Includes.h"

// debug timer
static ChronoClock timer;

// Test scene function to demonstrate rendering with user-controlled transformations
// No input variables
void sceneTest() {
	Renderer renderer;
	// create light source {direction, diffuse intensity, ambient intensity}
	Light L{ vec4(0.f, 1.f, 1.f, 0.f), colour(1.0f, 1.0f, 1.0f), colour(0.1f, 0.1f, 0.1f) };
	// camera is just a matrix
	matrix camera = matrix::makeIdentity(); // Initialize the camera with identity matrix

	bool running = true; // Main loop control variable

	std::vector<Mesh> scene; // Vector to store scene objects

	// Create a sphere and a rectangle mesh
	int totalX = 10, totalY = 10, totalZ = 5, space = 2;
	for (int i = 0; i < totalX; i++)
	{
		for (int j = 0; j < totalY; j++)
		{
			for (int k = 0; k < totalZ; k++)
			{
				//Mesh mesh = Mesh::makeCube(2);
				Mesh mesh = Mesh::makeSphere(1.f, 10, 10);
				mesh.world = matrix::makeTranslation((i - totalX / 2) * space, (j - totalY / 2) * space, -k * space - 4);
				scene.push_back(mesh);
			}
		}
	}

	float x = 0.0f, y = 0.0f, z = -4.0f; // Initial translation parameters

	// Main rendering loop
	while (running) {
		renderer.canvas.checkInput(); // Handle user input
		renderer.clear(); // Clear the canvas for the next frame

		timer.enable = renderer.canvas.keyPressed(VK_SPACE);
		timer.Start();

		// Handle user inputs for transformations
		if (renderer.canvas.keyPressed(VK_ESCAPE)) break;
		if (renderer.canvas.keyPressed('A')) x += 0.1f;
		if (renderer.canvas.keyPressed('D')) x += -0.1f;
		if (renderer.canvas.keyPressed('W')) z += 0.1f;
		if (renderer.canvas.keyPressed('S')) z += -0.1f;
		if (renderer.canvas.keyPressed('Q')) y += 0.1f;
		if (renderer.canvas.keyPressed('E')) y += -0.1f;

		// Apply transformations to the camera
		camera = matrix::makeTranslation(x, y, z);

		// Render each object in the scene
		for (auto& m : scene)
			render(renderer, &m, camera, L);

		renderer.present(); // Display the rendered frame

		timer.Stop();
		timer.Print();
	}
}