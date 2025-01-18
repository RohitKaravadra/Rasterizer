#include "Includes.h"

// Scene with a grid of cubes and a moving sphere
// No input variables
void scene2() {
	Renderer renderer;
	matrix camera = matrix::makeIdentity();
	Light L{ vec4(0.f, 1.f, 1.f, 0.f), color(1.0f, 1.0f, 1.0f), color(0.1f, 0.1f, 0.1f) };

	std::vector<Mesh*> scene;

	struct rRot { float x; float y; float z; }; // Structure to store random rotation parameters
	std::vector<rRot> rotations;

	RandomNumberGenerator& rng = RandomNumberGenerator::getInstance();

	// Create a grid of cubes with random rotations
	for (unsigned int y = 0; y < 6; y++) {
		for (unsigned int x = 0; x < 8; x++) {
			Mesh* m = new Mesh();
			*m = Mesh::makeCube(1.f);
			scene.push_back(m);
			m->world = matrix::makeTranslation(-7.0f + (static_cast<float>(x) * 2.f), 5.0f - (static_cast<float>(y) * 2.f), -8.f);
			rRot r{ rng.getRandomFloat(-.1f, .1f), rng.getRandomFloat(-.1f, .1f), rng.getRandomFloat(-.1f, .1f) };
			rotations.push_back(r);
		}
	}

	// Create a sphere and add it to the scene
	Mesh* sphere = new Mesh();
	*sphere = Mesh::makeSphere(1.0f, 10, 20);
	scene.push_back(sphere);
	float sphereOffset = -6.f;
	float sphereStep = 0.1f;
	sphere->world = matrix::makeTranslation(sphereOffset, 0.f, -6.f);

	auto start = std::chrono::high_resolution_clock::now();
	std::chrono::time_point<std::chrono::high_resolution_clock> end;
	int cycle = 0;

	bool running = true;
	while (running) {
		renderer.canvas.checkInput();
		if (renderer.canvas.keyPressed(VK_ESCAPE) || renderer.canvas.IsQuit()) break;

		renderer.clear();

		// Rotate each cube in the grid
		for (unsigned int i = 0; i < rotations.size(); i++)
			scene[i]->world = scene[i]->world * matrix::makeRotateXYZ(rotations[i].x, rotations[i].y, rotations[i].z);

		// Move the sphere back and forth
		sphereOffset += sphereStep;
		sphere->world = matrix::makeTranslation(sphereOffset, 0.f, -6.f);
		if (sphereOffset > 6.0f || sphereOffset < -6.0f) {
			sphereStep *= -1.f;
			if (++cycle % 2 == 0) {
				end = std::chrono::high_resolution_clock::now();
				std::cout << cycle / 2 << " " << std::chrono::duration<double, std::milli>(end - start).count() << "\n";
				start = std::chrono::high_resolution_clock::now();
			}
		}

		// update view projection matrix before rendering
		renderer.updateVP(camera);
		// render all objects in a scene
		//for (auto& m : scene)
		//	render(m, renderer, L);
		renderNew(scene, renderer, L);

		renderer.present();
	}

	for (auto& m : scene)
		delete m;
}