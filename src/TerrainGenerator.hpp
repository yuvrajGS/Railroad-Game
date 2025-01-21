#ifndef TERRAIN_GENERATOR_HPP
#define TERRAIN_GENERATOR_HPP

#include <GL/glew.h>
#include <vector>
#include <random>
#include <queue>
#include "GL/glaux.h"
#include "pixmap/RGBpixmap.h"

class TerrainGenerator {
private:
	static const int TERRAIN_RESOLUTION = 100;  // Grid size for terrain
	int winWidth, winHeight;
	GLuint textures[4];  // Water, grass, rock and track textures
	std::vector<int> p;  // Permutation table

	// Perlin noise helper functions
	double fade(double t);
	double lerp(double t, double a, double b);
	double grad(int hash, double x, double y);
	double noise(double x, double y);

	bool isValidPosition(int x, int y);
	bool hasGrassPath(std::pair<int, int> start, std::pair<int, int> end);
public:
	std::vector<std::vector<int>> terrain;      // 0 = water, 1 = grass, 2 = rock
	std::pair<int, int> startPos;
	std::pair<int, int> endPos;

	TerrainGenerator(int width, int height);
	void generateTerrain();
	void findValidPositions();
	void render();
	void setTextures(GLuint waterTex, GLuint grassTex, GLuint rockTex, GLuint trackTex);
	std::pair<int, int> windowToGrid(float x, float y);
	std::pair<float, float> gridToWindow(int gridX, int gridY);
	void setWindowDimensions(int width, int height);
};

#endif // TERRAIN_GENERATOR_HPP
