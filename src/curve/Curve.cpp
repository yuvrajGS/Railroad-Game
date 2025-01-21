#include "Curve.hpp"
#include <stdio.h>

Curve::Curve() {
    nPts = 0;
}

Curve::~Curve() {
}

void Curve::reset() {
    nPts = 0;
}

void Curve::drawCurve() {
    glLineWidth(2.0);
    glColor3f(1.0, 0.0, 1.0);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < nPts; i++) {
        glVertex2f(Pts[i].x, Pts[i].y);
    }
    glEnd();
}

void Curve::set2DView(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(left, right, bottom, top);

}
