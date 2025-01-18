#include "Includes.h"

// Function to render a scene with multiple objects and dynamic transformations
// No input variables
void scene1() {
	Renderer renderer;
	matrix camera;
	Light L{ vec4(0.f, 1.f, 1.f, 0.f), color(1.0f, 1.0f, 1.0f), color(0.1f, 0.1f, 0.1f) };

	bool running = true;

	std::vector<Mesh*> scene;

	// Create a scene of 40 cubes with random rotations
	for (unsigned int i = 0; i < 20; i++) {
		Mesh* m = new Mesh();
		*m = Mesh::makeCube(1.f);
		m->world = matrix::makeTranslation(-2.0f, 0.0f, (-3 * static_cast<float>(i))) * makeRandomRotation();
		scene.push_back(m);
		m = new Mesh();
		*m = Mesh::makeCube(1.f);
		m->world = matrix::makeTranslation(2.0f, 0.0f, (-3 * static_cast<float>(i))) * makeRandomRotation();
		scene.push_back(m);
	}

	float zoffset = 8.0f; // Initial camera Z-offset
	float step = -0.1f;  // Step size for camera movement

	auto start = std::chrono::high_resolution_clock::now();
	std::chrono::time_point<std::chrono::high_resolution_clock> end;
	int cycle = 0;

	// Main rendering loop
	while (running) {
		renderer.canvas.checkInput();
		if (renderer.canvas.keyPressed(VK_ESCAPE) || renderer.canvas.IsQuit()) break;

		renderer.clear();

		camera = matrix::makeTranslation(0, 0, -zoffset); // Update camera position

		// Rotate the first two cubes in the scene
		scene[0]->world = scene[0]->world * matrix::makeRotateXYZ(0.1f, 0.1f, 0.0f);
		scene[1]->world = scene[1]->world * matrix::makeRotateXYZ(0.0f, 0.1f, 0.2f);


		zoffset += step;
		if (zoffset < -60.f || zoffset > 8.f) {
			step *= -1.f;
			if (++cycle % 2 == 0) {
				end = std::chrono::high_resolution_clock::now();
				std::cout << cycle / 2 << " " << std::chrono::duration<double, std::milli>(end - start).count() << "\n";
				start = std::chrono::high_resolution_clock::now();
			}
		}

		// update view projection matrix before rendering
		renderer.updateVP(camera);
		// render all objects in a scene
		for (auto& m : scene)
			render(m, renderer, L);
		renderer.present();
	}

	for (auto& m : scene)
		delete m;
}