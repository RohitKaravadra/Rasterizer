#include "Includes.h"

// debug timer
static ChronoTimer timer;

// Test scene function to demonstrate rendering with user-controlled transformations
// No input variables
void scene3() {
	Renderer renderer;

	// create light source {direction, diffuse intensity, ambient intensity}
	Light L{ vec4(0.f, 1.f, 1.f, 0.f), color(1.0f, 1.0f, 1.0f), color(0.1f, 0.1f, 0.1f) };

	// camera is just a matrix
	matrix camera = matrix::makeIdentity(); // Initialize the camera with identity matrix

	RandomNumberGenerator& rng = RandomNumberGenerator::getInstance();

	std::vector<Mesh*> scene; // Vector to store scene objects

	struct rRot { float x; float y; float z; }; // Structure to store random rotation parameters
	std::vector<rRot> rotations;

	// Create a sphere and a rectangle mesh
	int totalX = 20, totalY = 20, totalZ = 20, space = 2;
	for (int i = 0; i < totalX; i++)
	{
		for (int j = 0; j < totalY; j++)
		{
			for (int k = 0; k < totalZ; k++)
			{
				//Mesh mesh = Mesh::makeCube(2);
				Mesh* mesh = new Mesh();
				*mesh = Mesh::makeSphere(1.f, 10, 10);
				//*mesh = Mesh::makeCube(1);
				mesh->world = matrix::makeTranslation((i - totalX / 2) * space, (j - totalY / 2) * space, -k * space - 4);
				scene.push_back(mesh);
				rRot r{ rng.getRandomFloat(-.1f, .1f), rng.getRandomFloat(-.1f, .1f), rng.getRandomFloat(-.1f, .1f) };
				rotations.push_back(r);
			}
		}
	}

	float x = 0.0f, y = 0.0f, z = -4.0f; // Initial translation parameters

	bool running = true; // Main loop control variable
	// Main rendering loop
	while (running) {
		renderer.canvas.checkInput(); // Handle user input
		if (renderer.canvas.keyPressed(VK_ESCAPE) || renderer.canvas.IsQuit()) break;

		renderer.clear(); // Clear the canvas for the next frame

		//timer.enable = renderer.canvas.keyPressed(VK_SPACE);

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

		// Rotate each cube in the grid
		for (unsigned int i = 0; i < rotations.size(); i++)
			scene[i]->world = scene[i]->world * matrix::makeRotateXYZ(rotations[i].x, rotations[i].y, rotations[i].z);

		// update view projection matrix before rendering
		renderer.updateVP(camera);

		timer.reset();

		render(scene, renderer, L);

		timer.elapsed();

		renderer.present(); // Display the rendered frame
	}

	for (auto& m : scene)
		delete m;
}