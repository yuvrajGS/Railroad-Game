#include "TerrainGenerator.hpp"
#include <chrono>
double TerrainGenerator::fade(double t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

double TerrainGenerator::lerp(double t, double a, double b) {
	return a + t * (b - a);
}

double TerrainGenerator::grad(int hash, double x, double y) {
	int h = hash & 15;
	double u = h < 8 ? x : y;
	double v = h < 4 ? y : (h == 12 || h == 14 ? x : 0);
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double TerrainGenerator::noise(double x, double y) {
	int X = (int)floor(x) & 255;
	int Y = (int)floor(y) & 255;

	x -= floor(x);
	y -= floor(y);

	double u = fade(x);
	double v = fade(y);

	int A = p[X] + Y;
	int B = p[X + 1] + Y;

	return lerp(v,
			   lerp(u, grad(p[A], x, y), grad(p[B], x-1, y)),
			   lerp(u, grad(p[A + 1], x, y-1), grad(p[B + 1], x-1, y-1)));
}

bool TerrainGenerator::isValidPosition(int x, int y) {
	return x >= 0 && x < TERRAIN_RESOLUTION && y >= 0 && y < TERRAIN_RESOLUTION;
}

bool TerrainGenerator::hasGrassPath(std::pair<int, int> start, std::pair<int, int> end) {
	if (terrain[start.first][start.second] != 1 || terrain[end.first][end.second] != 1) {
		return false;
	}

	std::vector<std::vector<bool>> visited(TERRAIN_RESOLUTION,
										 std::vector<bool>(TERRAIN_RESOLUTION, false));
	std::queue<std::pair<int, int>> q;
	q.push(start);
	visited[start.first][start.second] = true;

	const int dx[] = {-1, 0, 1, 0};
	const int dy[] = {0, 1, 0, -1};

	while (!q.empty()) {
		auto current = q.front();
		q.pop();

		if (current == end) {
			return true;
		}

		for (int i = 0; i < 4; i++) {
			int newX = current.first + dx[i];
			int newY = current.second + dy[i];

			if (isValidPosition(newX, newY) && !visited[newX][newY] &&
				terrain[newX][newY] == 1) {
				visited[newX][newY] = true;
				q.push({newX, newY});
			}
		}
	}
	return false;
}

TerrainGenerator::TerrainGenerator(int width, int height) : winWidth(width), winHeight(height) {
	// Initialize permutation table
	p.resize(512);
	std::iota(p.begin(), p.begin() + 256, 0);
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::mt19937 gen(seed);
	std::shuffle(p.begin(), p.begin() + 256, gen);
	std::copy(p.begin(), p.begin() + 256, p.begin() + 256);

	terrain.resize(TERRAIN_RESOLUTION, std::vector<int>(TERRAIN_RESOLUTION));
}

void TerrainGenerator::generateTerrain() {
	const double scale = 0.1;  // Adjust for different terrain features size

	// Generate terrain using Perlin noise
	for (int i = 0; i < TERRAIN_RESOLUTION; i++) {
		for (int j = 0; j < TERRAIN_RESOLUTION; j++) {
			double value = noise(i * scale, j * scale);
			value = (value + 1.0) / 2.0;  // Normalize to [0,1]

			// Classify into terrain types
			if (value < 0.25) {
				terrain[i][j] = 0;  // water
			} else if (value < 0.50) {
				terrain[i][j] = 1;  // grass
			} else {
				terrain[i][j] = 2;  // Rock
			}
		}
	}

	// Find valid start and end positions
	findValidPositions();
}

void TerrainGenerator::findValidPositions() {
	const int MIN_DISTANCE = TERRAIN_RESOLUTION / 2;  // Minimum distance between start and end

	for (int attempts = 0; attempts < 1000; attempts++) {
		// Randomly select start position on grass
		int startX = std::rand() % TERRAIN_RESOLUTION;
		int startY = std::rand() % TERRAIN_RESOLUTION;

		if (terrain[startX][startY] != 1) continue;

		// Try to find a valid end position
		for (int endX = 0; endX < TERRAIN_RESOLUTION; endX++) {
			for (int endY = 0; endY < TERRAIN_RESOLUTION; endY++) {
				if (terrain[endX][endY] != 1) continue;

				// Check distance
				double distance = std::sqrt(std::pow(endX - startX, 2) +
										 std::pow(endY - startY, 2));

				if (distance >= MIN_DISTANCE) {
					// Check if there's a valid path
					if (hasGrassPath({startX, startY}, {endX, endY})) {
						startPos = {startX, startY};
						endPos = {endX, endY};
						return;
					}
				}
			}
		}
	}
	// If no valid positions found, set default positions
	startPos = {TERRAIN_RESOLUTION/4, TERRAIN_RESOLUTION/4};
	endPos = {3*TERRAIN_RESOLUTION/4, 3*TERRAIN_RESOLUTION/4};
}

void TerrainGenerator::render() {
	glEnable(GL_TEXTURE_2D);

	// Reset color to white to ensure proper texture colors
	glColor3f(1.0f, 1.0f, 1.0f);

	float cellWidth = (float)winWidth / TERRAIN_RESOLUTION;
	float cellHeight = (float)winHeight / TERRAIN_RESOLUTION;

	for (int i = 0; i < TERRAIN_RESOLUTION; i++) {
		for (int j = 0; j < TERRAIN_RESOLUTION; j++) {
			float x1 = -winWidth/2 + i * cellWidth;
			float x2 = x1 + cellWidth;
			float y1 = -winHeight/2 + j * cellHeight;
			float y2 = y1 + cellHeight;

			// Select texture based on terrain type
			glBindTexture(GL_TEXTURE_2D, textures[terrain[i][j]]);

			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x1, y1);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x2, y1);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(x2, y2);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(x1, y2);
			glEnd();
		}
	}

	glDisable(GL_TEXTURE_2D);

	// Draw start and end positions
	glPointSize(10.0f);
	glBegin(GL_POINTS);
	glColor3f(0.0f, 1.0f, 0.0f);  // Green for start
	glVertex2f(-winWidth/2 + startPos.first * cellWidth + cellWidth/2,
			  -winHeight/2 + startPos.second * cellHeight + cellHeight/2);
	glColor3f(1.0f, 0.0f, 0.0f);  // Red for end
	glVertex2f(-winWidth/2 + endPos.first * cellWidth + cellWidth/2,
			  -winHeight/2 + endPos.second * cellHeight + cellHeight/2);
	glEnd();
}

void TerrainGenerator::setTextures(GLuint waterTex, GLuint grassTex, GLuint rockTex, GLuint trackTex) {
	textures[0] = waterTex;
	textures[1] = grassTex;
	textures[2] = rockTex;
	textures[3] = trackTex;
}

// Convert window coordinates to terrain grid coordinates
std::pair<int, int> TerrainGenerator::windowToGrid(float x, float y) {
	int gridX = (int)((x + winWidth/2) / (float)winWidth * TERRAIN_RESOLUTION);
	int gridY = (int)((y + winHeight/2) / (float)winHeight * TERRAIN_RESOLUTION);
	return {gridX, gridY};
}

// Convert grid coordinates to window coordinates
std::pair<float, float> TerrainGenerator::gridToWindow(int gridX, int gridY) {
	float x = -winWidth/2 + (gridX + 0.5f) * ((float)winWidth / TERRAIN_RESOLUTION);
	float y = -winHeight/2 + (gridY + 0.5f) * ((float)winHeight / TERRAIN_RESOLUTION);
	return {x, y};
}

void TerrainGenerator::setWindowDimensions(int width, int height) {
    winWidth = width;
    winHeight = height;
}
