#ifndef BSPLINE_H
#define BSPLINE_H

#include <GL/glew.h>
#include "../Point.hpp"
#include "../Vector.hpp"
#include "../curve/Curve.hpp"
#include "../TerrainGenerator.hpp"
#include <GL/glut.h>
#include <cmath>
#include <vector>
#define M_PI 3.14159265358979323846

class BSpline : public Curve {
public:
    GLint nCtrlPts;    // Number of control points
    Point ctrlPts[10]; // Array to store control points, up to 10
    GLint degree;      // Degree of the B-spline
    GLint resolution;
    std::vector<GLfloat> knots;   // Knot vector
    std::vector<GLfloat> weights; // Store the weights of the control points
    float trainPosition;     // Parameter value along the curve (0 to 1)
	float trainSpeed;        // Speed of train movement
	float trainSize;         // Size of the train quad
	GLuint trainTextureID;   // Texture ID for the train
	int count;

    BSpline();
    void reset();
    void computeKnots();                           // Compute the knot vector
    GLfloat basis(GLint i, GLint k, GLfloat u);    // Recursive B-spline basis function
    void computeBSplinePt(GLfloat u, Point *bsPt); // Compute B-spline point at parameter u
    void computeBSplinePts();                      // Compute points for the B-spline curve
    void drawCPts();
    void displayCPts();
    void display();
    void drawCurve();
    void findPointSelected(int xMouse, int yMouse, int &selectedPointIdx);
    bool checkCollisions();
    bool checkStartAndEndPositions();
    void initTrainAnimation(GLuint textureID);
	void updateTrainPosition(float deltaTime);
	void drawTrain();
	Point getTangent(float u);
};

#endif
