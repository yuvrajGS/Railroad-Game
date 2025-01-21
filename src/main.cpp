/*
 *  Author: Yuvraj Singh
 *  Version: 2024-11-20
 */

#include <GL/glew.h>
#include "GL/glaux.h"
#include "glsl/Angel.h"
#include "pixmap/RGBpixmap.h"
#include "TerrainGenerator.hpp"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "Menu.hpp"
#include "curve/BSpline.hpp"

GLint winWidth = 800, winHeight = 800;

GLint displayOption = 0;
GLint moveControlMode = 0;

RGBpixmap pix[6]; /* pixmaps textures */
BSpline myBSpline;    /* Bezier curve object */

GLint selectedCtrlPt = -1;
TerrainGenerator* terrainGen = nullptr;

std::pair<float, float> startPos, endPos;

extern GLint score[4];
GLuint textureIDs[6];
GLfloat lastFrameTime = 0.0f;
GLfloat deltaTime = 0.0f;

void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    /* load texture bitmap */
    glGenTextures(5, textureIDs);
    pix[0].readBMPFile("texture/water.bmp");
    pix[0].setTexture(textureIDs[0]);

    pix[1].readBMPFile("texture/rock.bmp");
    pix[1].setTexture(textureIDs[1]);

    pix[2].readBMPFile("texture/grass.bmp");
    pix[2].setTexture(textureIDs[2]);

    pix[3].readBMPFile("texture/score.bmp");
    pix[3].setTexture(textureIDs[3]);

    pix[4].readBMPFile("texture/track.bmp");
	pix[4].setTexture(textureIDs[4]);

	pix[5].readBMPFile("texture/train.bmp");
	pix[5].setTexture(textureIDs[5]);

    terrainGen = new TerrainGenerator(winWidth, winHeight);
	terrainGen->setTextures(textureIDs[0], textureIDs[2], textureIDs[1], textureIDs[4]);  // Set texture IDs
	terrainGen->generateTerrain();

	startPos = terrainGen->gridToWindow(terrainGen->startPos.first, terrainGen->startPos.second);
	endPos = terrainGen->gridToWindow(terrainGen->endPos.first, terrainGen->endPos.second);

	myBSpline.ctrlPts[myBSpline.nCtrlPts].x = startPos.first;
	myBSpline.ctrlPts[myBSpline.nCtrlPts].y = startPos.second;
	myBSpline.ctrlPts[myBSpline.nCtrlPts].z = 0;
	myBSpline.nCtrlPts++;
	myBSpline.nPts = (myBSpline.nCtrlPts - myBSpline.degree) * myBSpline.resolution;
	myBSpline.computeKnots();      // Update the knot vector
	myBSpline.computeBSplinePts(); // Recompute curve points
	glutPostRedisplay();           // Redraw to reflect the new point

	myBSpline.initTrainAnimation(textureIDs[5]);

}

void renderBackgroundImage() {
    glEnable(GL_TEXTURE_2D);  // Enable texturing
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);  // Reset color to white
	glDisable(GL_BLEND);  // Disable blending if not needed
    glBindTexture(GL_TEXTURE_2D, textureIDs[3]);  // Bind the background texture

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-winWidth/2, -winHeight/2);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(winWidth/2, -winHeight/2);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(winWidth/2, winHeight/2);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(-winWidth/2, winHeight/2);
    glEnd();

    glDisable(GL_TEXTURE_2D);  // Disable texturing for further drawing
}

// Helper to render text at a specific position
void renderText(GLfloat x, GLfloat y, const char* label, GLint value) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "%s %d", label, value);

    glRasterPos2f(x, y);  // Position the text
    for (const char* c = buffer; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

// Helper to render the score overlay
void renderScoreOverlay() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, winWidth, 0.0, winHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    void* font = GLUT_BITMAP_9_BY_15;
    const float labelX = 200;
    const float scoreX = labelX + 300;
    float y = 400;

    // Calculate box dimensions with padding
    const float padding = 20.0f;
    const float boxTop = y + padding;
    const float boxBottom = y - 140 - padding;
    const float boxLeft = labelX - padding;
    const float boxRight = scoreX + 50 + padding;

    // Draw translucent box
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw box background
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(boxLeft, boxBottom);
    glVertex2f(boxRight, boxBottom);
    glVertex2f(boxRight, boxTop);
    glVertex2f(boxLeft, boxTop);
    glEnd();

    // Optional: Draw box border
    glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(boxLeft, boxBottom);
    glVertex2f(boxRight, boxBottom);
    glVertex2f(boxRight, boxTop);
    glVertex2f(boxLeft, boxTop);
    glEnd();

    // Render text
    glColor3f(1.0f, 1.0f, 1.0f);
    char buffer[128];

    // Base Distance Score
    glRasterPos2f(labelX, y);
    glutBitmapString(font, (const unsigned char*)"Base Distance Score");
    glRasterPos2f(scoreX, y);
    snprintf(buffer, sizeof(buffer), "  %d", score[0]);
    glutBitmapString(font, (const unsigned char*)buffer);

    // Control Point Bonus
    y -= 30;
    glRasterPos2f(labelX, y);
    glutBitmapString(font, (const unsigned char*)"Control Point Bonus");
    glRasterPos2f(scoreX, y);
    snprintf(buffer, sizeof(buffer), "+ %d", score[1]);
    glutBitmapString(font, (const unsigned char*)buffer);

    // Smoothness Bonus
    y -= 30;
    glRasterPos2f(labelX, y);
    glutBitmapString(font, (const unsigned char*)"Smoothness Bonus");
    glRasterPos2f(scoreX, y);
    snprintf(buffer, sizeof(buffer), "+ %d", score[2]);
    glutBitmapString(font, (const unsigned char*)buffer);

    // Draw line above final score
    y -= 30;
    glBegin(GL_LINES);
    glVertex2f(scoreX, y + 10);
    glVertex2f(scoreX + 50, y + 10);
    glEnd();

    // Final Score
    y -= 30;
    glRasterPos2f(labelX, y);
    glutBitmapString(font, (const unsigned char*)"Final Score");
    glRasterPos2f(scoreX, y);
    snprintf(buffer, sizeof(buffer), "%d", score[3]);
    glutBitmapString(font, (const unsigned char*)buffer);

    glDisable(GL_BLEND);

    // Restore matrices
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// Helper to render a single line of text
void renderText(GLfloat x, GLfloat y, const char* text) {
    glRasterPos2f(x, y);  // Position the text
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);  // Render each character
    }
}

void renderInstructionText() {
    // Set up 2D rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 800.0, 0.0, 600.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Render text in the bottom-right corner
    GLfloat startX = 550.0f;
    GLfloat startY = 50.0f;

    glColor3f(1.0f,1.0f,1.0f);
    renderText(startX, startY + 40.0f, "Move control points by selecting them.");
    renderText(startX, startY + 20.0f, "Adjust control point weights:");
    renderText(startX, startY, "w = increase weight");
    renderText(startX, startY - 20.0f, "s = decrease weight");
    renderText(startX, startY - 40.0f, "r = reset weight");

    // Restore matrices
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();  // Reset modelview matrix

    switch (displayOption) {
        case 0: {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Render terrain
            terrainGen->render();

            // Disable texturing for B-spline rendering
            glDisable(GL_TEXTURE_2D);

            // Render the B-spline
            myBSpline.display();

            glDisable(GL_BLEND);
            break;
        }
        case 1: {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Render terrain
            terrainGen->render();

            // Disable texturing for B-spline rendering
            glDisable(GL_TEXTURE_2D);

            // Render the B-spline
            myBSpline.display();

            renderInstructionText();

            glDisable(GL_BLEND);
            break;
        }
        case 2: {
        	glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Render terrain
			terrainGen->render();

			// Disable texturing for B-spline rendering
			glDisable(GL_TEXTURE_2D);
			// Render the B-spline
			myBSpline.display();

			glDisable(GL_BLEND);
        	GLfloat currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
			deltaTime = currentTime - lastFrameTime;
			lastFrameTime = currentTime;

			myBSpline.updateTrainPosition(deltaTime);
			myBSpline.drawTrain();
			break;
		}
        case 3: {
            // Results screen
            renderBackgroundImage();
            // Render the score overlay
            renderScoreOverlay();
            break;
        }
    }

    glFlush();
    glutSwapBuffers();
}

void timer(int value) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);  // ~60 FPS
}

void winReshapeFcn(GLint width, GLint height) {
	if (width <= 0 || height <= 0) return;  // Prevent invalid dimensions

	// Update the viewport to the new dimensions
	glViewport(0, 0, width, height);

	// Update projection matrix with the new dimensions
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-width / 2, width / 2, -height / 2, height / 2);

	// Reset modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Update global window dimensions
	winWidth = width;
	winHeight = height;

	// Update TerrainGenerator's window dimensions
	terrainGen->setWindowDimensions(width, height);
	startPos = terrainGen->gridToWindow(terrainGen->startPos.first, terrainGen->startPos.second);
	endPos = terrainGen->gridToWindow(terrainGen->endPos.first, terrainGen->endPos.second);

	glutPostRedisplay();
}
void mouseActionFcn(int button, int state, int xMouse, int yMouse) {
    // Convert mouse coordinates to OpenGL coordinates
    float x = xMouse - winWidth / 2;
    float y = winHeight / 2 - yMouse;
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            selectedCtrlPt = -1; // Reset selected control point

            if (moveControlMode == 1) {
                // Try to find the control point to move
                myBSpline.findPointSelected(x, y, selectedCtrlPt);
            } else if (moveControlMode == 0) {
                // Add a new control point
                if (myBSpline.nCtrlPts < 10) {
                    myBSpline.ctrlPts[myBSpline.nCtrlPts].x = x;
                    myBSpline.ctrlPts[myBSpline.nCtrlPts].y = y;
                    myBSpline.ctrlPts[myBSpline.nCtrlPts].z = 0;
                    myBSpline.nCtrlPts++;
                    myBSpline.nPts = (myBSpline.nCtrlPts - myBSpline.degree) * myBSpline.resolution;
                    myBSpline.computeKnots();      // Update the knot vector
                    myBSpline.computeBSplinePts(); // Recompute curve points
                    glutPostRedisplay();           // Redraw to reflect the new point
                } else {
                    printf("Reached the max number of control points \n");
                }
            }
        }

        if (state == GLUT_UP) {
            // Move the selected control point
            if (moveControlMode == 1 && selectedCtrlPt != -1) {
                myBSpline.ctrlPts[selectedCtrlPt].x = x;
                myBSpline.ctrlPts[selectedCtrlPt].y = y;
                myBSpline.computeKnots();      // Recompute the knot vector
                myBSpline.computeBSplinePts(); // Recompute the curve points
                glutPostRedisplay();           // Redraw to reflect changes
            }
        }
    }
}

void keyboardActionFcn(unsigned char key, int x, int y) {
    if (key == 'w' && selectedCtrlPt != -1 && myBSpline.weights[selectedCtrlPt] < 5.0f) { // Let 'w' be the key for increasing the weight
		myBSpline.weights[selectedCtrlPt] += 0.1f; // Increase the weight for the selected point
		myBSpline.computeBSplinePts();             // Recompute the curve
		glutPostRedisplay();
    } else if (key == 's' && selectedCtrlPt != -1 && myBSpline.weights[selectedCtrlPt] > 0.1f) { // Let 's' be the key for decreasing the weight
		myBSpline.weights[selectedCtrlPt] -= 0.1f; // Decrease the weight for the selected point
		myBSpline.computeBSplinePts();             // Recompute the curve
		glutPostRedisplay();
    } else if (key == 'r' && selectedCtrlPt != -1) { // Reset selected weight
		myBSpline.weights[selectedCtrlPt] = 1.0f; // Decrease the weight for the selected point
		myBSpline.computeBSplinePts();             // Recompute the curve
		glutPostRedisplay();

    }
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    glutInitWindowSize(winWidth, winHeight);
    glutCreateWindow("2D RailRoad Game (Yuvraj Singh) ");

    menu();
    init();
    glutReshapeFunc(winReshapeFcn);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardActionFcn);
    glutMouseFunc(mouseActionFcn);
    glutTimerFunc(0, timer, 0);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    glutMainLoop();
    delete terrainGen;
    return 0;
}
