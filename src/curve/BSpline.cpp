#include "BSpline.hpp"

extern TerrainGenerator* terrainGen;
extern std::pair<float, float> startPos, endPos;
extern GLint displayOption;

BSpline::BSpline() {
    reset();
}

void BSpline::reset() {
    degree = 3; // Default degree for cubic B-spline
    knots.clear();
    resolution = 20;                          // Default resolution (points per segment)
    weights = std::vector<GLfloat>(10, 1.0f); // Initialize all weights to 1.0
    nCtrlPts = 0;
    nPts = 0;
    count = 0;
}

void BSpline::computeKnots() {
    GLint m = nCtrlPts + degree + 1;
    knots.resize(m);

    // For a clamped B-spline:
    // First (degree + 1) knots are 0
    for (int i = 0; i <= degree; i++) {
        knots[i] = 0.0f;
    }

    // Internal knots
    int internal_knots = m - 2 * (degree + 1);
    for (int i = 1; i <= internal_knots; i++) {
        knots[degree + i] = (float)i / (internal_knots + 1);
    }

    // Last (degree + 1) knots are 1
    for (int i = m - degree - 1; i < m; i++) {
        knots[i] = 1.0f;
    }
}

GLfloat BSpline::basis(GLint i, GLint k, GLfloat u) {
    if (k == 0) {
        if (i == knots.size() - k - 2 && u == knots[knots.size() - 1]) {
            return 1.0f;
        }
        return (u >= knots[i] && u < knots[i + 1]) ? 1.0f : 0.0f;
    }

    GLfloat left = 0.0f, right = 0.0f;

    // Handle division by zero cases explicitly
    GLfloat d1 = knots[i + k] - knots[i];
    GLfloat d2 = knots[i + k + 1] - knots[i + 1];

    if (d1 > 0.0f) {
        left = ((u - knots[i]) / d1) * basis(i, k - 1, u);
    }
    if (d2 > 0.0f) {
        right = ((knots[i + k + 1] - u) / d2) * basis(i + 1, k - 1, u);
    }

    return left + right;
}

void BSpline::computeBSplinePt(GLfloat u, Point *bsPt) {
    bsPt->x = 0.0;
    bsPt->y = 0.0;
    bsPt->z = 0.0;

    GLfloat sumWeights = 0.0f; // Sum of weighted basis functions

    for (int i = 0; i < nCtrlPts; i++) {
        GLfloat b = basis(i, degree, u); // Basis function
        GLfloat weight = weights[i];     // Weight for the control point

        // Weighted sum of control points
        bsPt->x += weight * b * ctrlPts[i].x;
        bsPt->y += weight * b * ctrlPts[i].y;
        bsPt->z += weight * b * ctrlPts[i].z;

        // Sum of the weights
        sumWeights += weight * b;
    }

    // Normalize the result (divide by the sum of weights)
    bsPt->x /= sumWeights;
    bsPt->y /= sumWeights;
    bsPt->z /= sumWeights;
}

void BSpline::computeBSplinePts() {
    if (nPts < 2) {
        nPts = 2;
    }

    // Use the full parameter range [0, 1]
    GLfloat uStart = 0.0f;
    GLfloat uEnd = 1.0f;

    GLfloat uStep = (uEnd - uStart) / (nPts - 1);

    for (int i = 0; i < nPts; i++) {
        GLfloat u;
        if (i == nPts - 1) {
            u = uEnd - 1e-6f;
        } else {
            u = uStart + i * uStep;
        }
        computeBSplinePt(u, &Pts[i]);
    }
}

void BSpline::drawCPts() {
	GLfloat currentColor[4];
	glGetFloatv(GL_CURRENT_COLOR, currentColor);  // Save current color

    glPointSize(8.0);
    glColor3f(1.0, 1.0, 0.0);
    glBegin(GL_POINTS);
    for (int i = 0; i < nCtrlPts; i++) {
        glVertex2f(ctrlPts[i].x, ctrlPts[i].y);
    }
    glEnd();

    glColor4fv(currentColor);  // Restore original color
}

void BSpline::displayCPts() {
    drawCPts();
}

void BSpline::display() {
    drawCPts();
    drawCurve();
}

void BSpline::drawCurve() {
	GLfloat currentColor[4];
	glGetFloatv(GL_CURRENT_COLOR, currentColor);  // Save current color

    glLineWidth(2.0);
    glColor3f(1.0, 1.0, 0.0);

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < nPts; i++) {
        if (Pts[i].x == 0 && Pts[i].y == 0)
            continue; // Skip invalid points
        glVertex2f(Pts[i].x, Pts[i].y);
    }
    glEnd();

    glColor4fv(currentColor);  // Restore original color
}

void BSpline::findPointSelected(int xMouse, int yMouse, int &selectedPointIdx) {
    float threshold = 20.0f; // Sensitivity to click within a radius of the control point

    selectedPointIdx = -1; // Reset selection

    for (int i = 0; i < nCtrlPts; i++) {
        // Calculate distance between mouse and control point
        float dx = xMouse - ctrlPts[i].x;
        float dy = yMouse - ctrlPts[i].y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < threshold) {
            selectedPointIdx = i; // Point selected
            return;               // Stop after the first match
        }
    }
}

std::vector<std::pair<int, int>> bresenham(int x0, int y0, int x1, int y1) {
    std::vector<std::pair<int, int>> points;
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        points.emplace_back(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }

    return points;
}


bool BSpline::checkCollisions() {
	if (nCtrlPts >=4){
		for (int i = 0; i < nPts - 1; i++) {
			std::pair<float, float> coord0;
			std::pair<float, float> coord1;
			// Map start and end points of the segment to grid coordinates
			coord0 = terrainGen->windowToGrid(Pts[i].x, Pts[i].y);
			coord1 = terrainGen->windowToGrid(Pts[i + 1].x, Pts[i + 1].y);

			// Use Bresenham's algorithm to traverse the grid cells
			std::vector<std::pair<int, int>> cells = bresenham(coord0.first, coord0.second, coord1.first, coord1.second);

			// Check each cell for intersection
			for (const auto& grid : cells) {
				// Ensure indices are within bounds
				if (grid.first >= 0 && grid.first < terrainGen->terrain.size() && grid.second >= 0 && grid.second < terrainGen->terrain[0].size()) {
					int terrian = terrainGen->terrain[grid.first][grid.second];
					if (terrian == 0 || terrian == 2) {
						return true;  // Intersection found
					}else if (terrian == 1){
						// Replace grass with track texture
						terrainGen->terrain[grid.first][grid.second] = 3; // Index for track texture
					}
				}
			}
		}
	}
	return false;  // No intersection
}

bool BSpline::checkStartAndEndPositions(){
	if (nCtrlPts < 2 ){
		return false;
	}

	float threshold = 20.0f;



	float startDX = startPos.first - ctrlPts[0].x;
	float startDY = startPos.second - ctrlPts[0].y;
	float startDistance = sqrt(startDX * startDX + startDY * startDY);

	float endDX = endPos.first - ctrlPts[nCtrlPts-1].x;
	float endDY = endPos.second - ctrlPts[nCtrlPts-1].y;
	float endDistance = sqrt(endDX * endDX + endDY * endDY);

	if (startDistance > threshold || endDistance > threshold) {
		return false;
	}


	return true;
}

void BSpline::initTrainAnimation(GLuint textureID) {
    trainPosition = 0.0f;
    trainSpeed = 0.2f;
    trainSize = 20.0f;
    trainTextureID = textureID;
}

Point BSpline::getTangent(float u) {
    Point p1, p2;
    float delta = 0.001f;

    // Get two nearby points to compute tangent
    computeBSplinePt(u, &p1);
    computeBSplinePt(std::min(u + delta, 1.0f), &p2);

    // Calculate direction vector
    Point tangent;
    tangent.x = p2.x - p1.x;
    tangent.y = p2.y - p1.y;

    // Normalize
    float length = sqrt(tangent.x * tangent.x + tangent.y * tangent.y);
    if (length > 0) {
        tangent.x /= length;
        tangent.y /= length;
    }

    return tangent;
}

void BSpline::updateTrainPosition(float deltaTime) {
    trainPosition += trainSpeed * deltaTime;
    if (trainPosition >= 1.0f) {
        trainPosition = 0.0f;
        count++;
    }
    if (count >=2){
    	displayOption =3;
    }
}

void BSpline::drawTrain() {
    if (nCtrlPts < 4) return;

    Point currentPos;
    computeBSplinePt(trainPosition, &currentPos);

    // Get tangent for rotation
    Point tangent = getTangent(trainPosition);
    float angle = atan2(tangent.y, tangent.x) * 180.0f / M_PI;

    GLfloat currentColor[4];
    glGetFloatv(GL_CURRENT_COLOR, currentColor);  // Save current color

    // Enable texturing
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, trainTextureID);

    // Save matrix state
    glPushMatrix();

    // Translate to current position and rotate
    glTranslatef(currentPos.x, currentPos.y, 0.0f);
    glRotatef(angle, 0.0f, 0.0f, 1.0f);

    // Draw textured quad
    glBegin(GL_QUADS);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);  // White to show texture properly

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-trainSize/2, -trainSize/2);

    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(trainSize/2, -trainSize/2);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(trainSize/2, trainSize/2);

    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-trainSize/2, trainSize/2);

    glEnd();

    // Restore matrix state
    glPopMatrix();

    // Disable texturing
    glDisable(GL_TEXTURE_2D);

    glColor4fv(currentColor);  // Restore original color
}
