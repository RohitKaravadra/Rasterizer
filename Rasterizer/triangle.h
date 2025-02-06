#pragma once

#include "mesh.h"
#include "colour.h"
#include "renderer.h"
#include "light.h"
#include <iostream>

// Simple support class for a 2D vector
class vec2D {
public:
	float x, y;

	// Default constructor initializes both components to 0
	vec2D() { x = y = 0.f; };

	// Constructor initializes components with given values
	vec2D(float _x, float _y) : x(_x), y(_y) {}

	// Constructor initializes components from a vec4
	vec2D(vec4 v) {
		x = v[0];
		y = v[1];
	}

	// Display the vector components
	void display() { std::cout << x << '\t' << y << std::endl; }

	// Overloaded subtraction operator for vector subtraction
	vec2D operator- (const vec2D& v) const {
		vec2D q;
		q.x = x - v.x;
		q.y = y - v.y;
		return q;
	}

	// ceil vec values
	void ceil()
	{
		x = std::ceil(x);
		y = std::ceil(y);
	}

	static vec2D _min(const vec2D& v1, const vec2D& v2) {
		return vec2D(min(v1.x, v2.x), min(v1.y, v2.y));
	}

	static vec2D _max(const vec2D& v1, const vec2D& v2) {
		return vec2D(max(v1.x, v2.x), max(v1.y, v2.y));
	}
};

// Class representing a triangle for rendering purposes
class triangle {

	Vertex v[3];       // Vertices of the triangle
	vec2D e[3];		   // Edges of the triangle
	color col[3];      // Colors for each vertex of the triangle

	float invArea;	   // 1 / Area of the triangle
public:
	// Constructor initializes the triangle with three vertices
	// Input Variables:
	// - v1, v2, v3: Vertices defining the triangle
	triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3) {
		// set vertices
		v[0] = v1; v[1] = v2; v[2] = v3;

		// calculate edges
		e[0] = vec2D(v[1].p - v[0].p);
		e[1] = vec2D(v[2].p - v[1].p);
		e[2] = vec2D(v[0].p - v[2].p);

		// clam colors
		col[0].clampColour();
		col[1].clampColour();
		col[2].clampColour();

		// Calculate the 2D area of the triangle
		float area = getCross(e[0], e[1]);
		invArea = area != 0 ? 1 / area : 100.f; // check for zero division

	}

	// Helper function to compute the cross product for barycentric coordinates
	// Input Variables:
	// - v1, v2: Edges defining the vector
	// - p: Point for which coordinates are being calculated
	float getCross(const vec2D& a, const vec2D& b) {
		return a.x * b.y - b.x * a.y;
	}

	// Compute barycentric coordinates for a given point
	// Input Variables:
	// - p: Point to check within the triangle
	// Output Variables:
	// - alpha, beta, gamma: Barycentric coordinates of the point
	// Returns true if the point is inside the triangle, false otherwise
	bool getCoordinates(vec2D p, float& alpha, float& beta, float& gamma) {
		alpha = getCross(e[0], p - v[1].p);
		beta = getCross(e[1], p - v[2].p);
		gamma = getCross(e[2], p - v[0].p);

		if (alpha < 0.f || beta < 0.f || gamma < 0.f) return false;
		return true;
	}

	// Template function to interpolate values using barycentric coordinates
	// Input Variables:
	// - alpha, beta, gamma: Barycentric coordinates
	// - a1, a2, a3: Values to interpolate
	// Returns the interpolated value
	template <typename T>
	T interpolate(float alpha, float beta, float gamma, T a1, T a2, T a3) {
		return (a1 * alpha) + (a2 * beta) + (a3 * gamma);
	}

	// Draw the triangle on the canvas
	// Input Variables:
	// - renderer: Renderer object for drawing
	// - L: Light object for shading calculations
	// - ka, kd: Ambient and diffuse lighting coefficients
	void draw(Renderer& renderer, const vec4& omega_i, const color& ambient, const color& diffuse) {

		// Skip very small triangles
		if (invArea > 1.f) return;

		int minX, minY, maxX, maxY;
		int canWidth = renderer.canvas.getWidth(), canHeight = renderer.canvas.getHeight();
		getBoundsWindow(canWidth, canHeight, minX, minY, maxX, maxY);

		// variable decalaration outside loops
		color c;
		vec4 normal;
		unsigned char finalColor[3];
		float depth, dot, alpha, beta, gamma;

		// Iterate over the bounding box and check each pixel
		for (int y = minY; y < maxY; y++) {

			// pre calculating buffer index for row
			int bufferIndex = y * canWidth;

			for (int x = minX; x < maxX; x++) {

				// Check if the pixel lies inside the triangle
				if (getCoordinates(vec2D(x, y), alpha, beta, gamma)) {

					alpha *= invArea;
					beta *= invArea;
					gamma *= invArea;

					// Interpolate color, depth, and normals
					depth = interpolate(beta, gamma, alpha, v[0].p[2], v[1].p[2], v[2].p[2]);
					// Perform Z-buffer test and apply shading
					if (depth > 0.01f && renderer.getDepth(bufferIndex + x) > depth) {

						c = interpolate(beta, gamma, alpha, v[0].rgb, v[1].rgb, v[2].rgb);

						normal = interpolate(beta, gamma, alpha, v[0].normal, v[1].normal, v[2].normal);
						normal.normalise();

						// typical shader begin
						dot = max(vec4::dot(omega_i, normal), 0.0f);
						c = c * dot * diffuse + ambient;
						// typical shader end

						c.toRGB(finalColor);

						renderer.drawAndSetDepth(bufferIndex + x, finalColor, depth);
					}
				}
			}
		}
	}

	// Draw the triangle on the canvas
	// Input Variables:
	// - renderer: Renderer object for drawing
	// - L: Light object for shading calculations
	// - ka, kd: Ambient and diffuse lighting coefficients
	void drawIncremental(Renderer& renderer, const vec4& omega_i, const color& ambient, const color& diffuse) {

		// Skip very small triangles
		if (invArea > 1.f) return;

		int minX, minY, maxX, maxY;
		int canWidth = renderer.canvas.getWidth(), canHeight = renderer.canvas.getHeight();
		getBoundsWindow(canWidth, canHeight, minX, minY, maxX, maxY);

		// variable decalaration outside loops
		color c;
		vec4 normal;
		float  depth, dot;
		unsigned char finalColor[3];

		vec2D p(minX, minY);

		// calculate starting value of barycentric coordinates
		float alpha0 = getCross(e[0], p - v[1].p) * invArea;
		float beta0 = getCross(e[1], p - v[2].p) * invArea;
		float gamma0 = getCross(e[2], p - v[0].p) * invArea;

		// calculate horozontal and verticle change in barycentric coordinates
		float deltaAlphaX = -e[0].y * invArea, deltaAlphaY = e[0].x * invArea;
		float deltaBetaX = -e[1].y * invArea, deltaBetaY = e[1].x * invArea;
		float deltaGammaX = -e[2].y * invArea, deltaGammaY = e[2].x * invArea;

		// set initial values of barycentric coordinates
		float alpha, beta, gamma;
		float alphaRow = alpha0, betaRow = beta0, gammaRow = gamma0;

		// Iterate over the bounding box and check each pixel
		for (int y = minY; y < maxY; y++) {

			// pre calculating buffer index for row
			int bufferIndex = y * canWidth;

			// set row barycentric coordinates
			alpha = alphaRow;
			beta = betaRow;
			gamma = gammaRow;

			for (int x = minX; x < maxX; x++) {

				// Check if the pixel lies inside the triangle
				if (alpha >= 0.f && beta >= 0.f && gamma >= 0.f) {
					// Interpolate color, depth, and normals
					depth = interpolate(beta, gamma, alpha, v[0].p[2], v[1].p[2], v[2].p[2]);
					// Perform Z-buffer test and apply shading
					if (depth > 0.01f && renderer.getDepth(bufferIndex + x) > depth) {

						c = interpolate(beta, gamma, alpha, v[0].rgb, v[1].rgb, v[2].rgb);

						normal = interpolate(beta, gamma, alpha, v[0].normal, v[1].normal, v[2].normal);
						normal.normalise();

						// typical shader begin
						dot = max(vec4::dot(omega_i, normal), 0.0f);
						c = c * dot * diffuse + ambient;
						// typical shader end

						c.toRGB(finalColor);

						renderer.drawAndSetDepth(bufferIndex + x, finalColor, depth);
					}
				}

				// horizontal increment of barycentric coordinates
				alpha += deltaAlphaX;
				beta += deltaBetaX;
				gamma += deltaGammaX;
			}

			// verticle increment of barycentric coordinates
			alphaRow += deltaAlphaY;
			betaRow += deltaBetaY;
			gammaRow += deltaGammaY;
		}
	}

	// Draw the triangle on the canvas
	// Input Variables:
	// - renderer: Renderer object for drawing
	// - L: Light object for shading calculations
	// - ka, kd: Ambient and diffuse lighting coefficients
	void drawIncrementalSIMD(Renderer& renderer, const vec4& omega_i, const color& ambient, const color& diffuse) {

		// Skip very small triangles
		if (invArea > 1.f) return;

		int minX, minY, maxX, maxY;
		int canWidth = renderer.canvas.getWidth(), canHeight = renderer.canvas.getHeight();
		getBoundsWindow(canWidth, canHeight, minX, minY, maxX, maxY);

		// variable decalaration outside loops
		color c;
		vec4 normal;
		float  depth, dot;
		unsigned char finalColor[3];

		vec2D p(minX, minY);

		// calculate starting value of barycentric coordinates
		float alpha0 = getCross(e[0], p - v[1].p) * invArea;
		float beta0 = getCross(e[1], p - v[2].p) * invArea;
		float gamma0 = getCross(e[2], p - v[0].p) * invArea;

		// calculate horozontal and verticle change in barycentric coordinates
		float deltaAlphaX = -e[0].y * invArea, deltaAlphaY = e[0].x * invArea;
		float deltaBetaX = -e[1].y * invArea, deltaBetaY = e[1].x * invArea;
		float deltaGammaX = -e[2].y * invArea, deltaGammaY = e[2].x * invArea;

		// set initial values of barycentric coordinates
		float alpha, beta, gamma;
		float alphaRow = alpha0, betaRow = beta0, gammaRow = gamma0;

		// Iterate over the bounding box and check each pixel
		for (int y = minY; y < maxY; y++) {

			// pre calculating buffer index for row
			int bufferIndex = y * canWidth;

			// set row barycentric coordinates
			alpha = alphaRow;
			beta = betaRow;
			gamma = gammaRow;

			for (int x = minX; x < maxX; x++) {

				// Check if the pixel lies inside the triangle
				if (alpha >= 0.f && beta >= 0.f && gamma >= 0.f) {
					// Interpolate color, depth, and normals
					depth = interpolate(beta, gamma, alpha, v[0].p[2], v[1].p[2], v[2].p[2]);
					// Perform Z-buffer test and apply shading
					if (depth > 0.01f && renderer.getDepth(bufferIndex + x) > depth) {

						c = interpolate(beta, gamma, alpha, v[0].rgb, v[1].rgb, v[2].rgb);

						normal = interpolate(beta, gamma, alpha, v[0].normal, v[1].normal, v[2].normal);
						normal.normalise();

						// typical shader begin
						dot = max(vec4::dot(omega_i, normal), 0.0f);
						c = c * dot * diffuse + ambient;
						// typical shader end

						c.toRGB(finalColor);

						renderer.drawAndSetDepth(bufferIndex + x, finalColor, depth);
					}
				}

				// horizontal increment of barycentric coordinates
				alpha += deltaAlphaX;
				beta += deltaBetaX;
				gamma += deltaGammaX;
			}

			// verticle increment of barycentric coordinates
			alphaRow += deltaAlphaY;
			betaRow += deltaBetaY;
			gammaRow += deltaGammaY;
		}
	}

	// Compute the 2D bounds of the triangle
	// Output Variables:
	// - minV, maxV: Minimum and maximum bounds in 2D space
	void getBounds(vec2D& minV, vec2D& maxV) {
		minV = vec2D(v[0].p);
		maxV = vec2D(v[0].p);
		for (unsigned int i = 1; i < 3; i++) {
			minV.x = min(minV.x, v[i].p[0]);
			minV.y = min(minV.y, v[i].p[1]);
			maxV.x = max(maxV.x, v[i].p[0]);
			maxV.y = max(maxV.y, v[i].p[1]);
		}
	}

	void getBoundsWindow(const int& width, const int& height, int& minX, int& minY, int& maxX, int& maxY) {

		vec2D minV = vec2D::_min(v[0].p, vec2D::_min(v[1].p, v[2].p));
		vec2D maxV = vec2D::_max(v[0].p, vec2D::_max(v[1].p, v[2].p));

		minV = vec2D::_max(minV, vec2D(0, 0));
		maxV = vec2D::_min(maxV, vec2D(width, height));
		maxV.ceil();

		minX = minV.x; minY = minV.y;
		maxX = maxV.x; maxY = maxV.y;
	}

	// Debugging utility to display the triangle bounds on the canvas
	// Input Variables:
	// - canvas: Reference to the rendering canvas
	void drawBounds(GamesEngineeringBase::Window& canvas) {
		vec2D minV, maxV;
		getBounds(minV, maxV);

		for (int y = (int)minV.y; y < (int)maxV.y; y++) {
			for (int x = (int)minV.x; x < (int)maxV.x; x++) {
				canvas.draw(x, y, 255, 0, 0);
			}
		}
	}

	// Debugging utility to display the coordinates of the triangle vertices
	void display() {
		for (unsigned int i = 0; i < 3; i++) {
			v[i].p.display();
		}
		std::cout << std::endl;
	}
};
