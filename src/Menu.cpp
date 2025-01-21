/*
 *  Author: Yuvraj Singh
 *  Version: 2024-11-20
 */
#include "Menu.hpp"

#include "curve/BSpline.hpp"
#include "TerrainGenerator.hpp"
#include "pixmap/RGBpixmap.h"

extern GLint selectedCtrlPt;
extern GLint displayOption;
extern GLint moveControlMode;
extern BSpline myBSpline; /* BSpline curve object */
extern std::pair<float, float> startPos, endPos;
extern TerrainGenerator* terrainGen;
extern GLint winWidth, winHeight;
extern GLuint textureIDs[6];

GLint score[4] = {0,0,0,0};

void menu() {
    GLint Curve_Surface_Menu = glutCreateMenu(curveSurfaceMenu);
    glutAddMenuEntry("Set control points", 1);
    glutAddMenuEntry("Adjust control points", 2);

    glutCreateMenu(mainMenu);
    glutAddMenuEntry("Reset", 1);
    glutAddSubMenu("Define Track", Curve_Surface_Menu);
    glutAddMenuEntry("Start Train", 2);
    glutAddMenuEntry("Generate New Terrain", 3);
    glutAddMenuEntry("Quit", 4);
}

void mainMenu(GLint option) {
    switch (option) {
    case 1:
        reset();
        break;
    case 2:
    	selectedCtrlPt = -1;
    	run();
		break;
    case 3:
    	newTerrain();
    	break;
    case 4:
        exit(0);
        break;
    }
    glutPostRedisplay();
}

void newTerrain(){
	terrainGen = new TerrainGenerator(winWidth, winHeight);
	terrainGen->setTextures(textureIDs[0], textureIDs[2], textureIDs[1], textureIDs[4]);  // Set texture IDs
	terrainGen->generateTerrain();

	startPos = terrainGen->gridToWindow(terrainGen->startPos.first, terrainGen->startPos.second);
	endPos = terrainGen->gridToWindow(terrainGen->endPos.first, terrainGen->endPos.second);

	reset();
}


GLint calculateSmoothnessBonus() {
    GLint maxBonus = 1750; // Start with a maximum bonus
    GLfloat totalAngleVariation = 0;

    for (int i = 1; i < myBSpline.nCtrlPts - 1; ++i) {
        // Vectors representing consecutive segments
        GLfloat dx1 = myBSpline.ctrlPts[i].x - myBSpline.ctrlPts[i - 1].x;
        GLfloat dy1 = myBSpline.ctrlPts[i].y - myBSpline.ctrlPts[i - 1].y;
        GLfloat dx2 = myBSpline.ctrlPts[i + 1].x - myBSpline.ctrlPts[i].x;
        GLfloat dy2 = myBSpline.ctrlPts[i + 1].y - myBSpline.ctrlPts[i].y;

        // Compute the angle between the segments
        GLfloat dot = dx1 * dx2 + dy1 * dy2;
        GLfloat mag1 = sqrt(dx1 * dx1 + dy1 * dy1);
        GLfloat mag2 = sqrt(dx2 * dx2 + dy2 * dy2);
        GLfloat angle = acos(dot / (mag1 * mag2));

        totalAngleVariation += fabs(angle); // Accumulate angle variations
    }

    // Penalize based on angle variation
    return maxBonus - static_cast<GLint>(totalAngleVariation * 150);
}


void run() {
	if (!myBSpline.checkCollisions() && myBSpline.checkStartAndEndPositions()){
		// Winner!

		displayOption = 2;
		GLfloat dx = startPos.first - endPos.first;
		GLfloat dy = startPos.second - endPos.second;
		GLint distance = ceil(sqrt(dx * dx + dy * dy));

		 // Base score from distance
		score[0] = distance * 6;

		// Control point bonus
		score[1] = 15000 / myBSpline.nCtrlPts;

        // Smoothness bonus
		score[2] = calculateSmoothnessBonus();

		// Total score
		score[3] = score[0]+score[1]+score[2];
	}

}


void curveSurfaceMenu(GLint option) {
    switch (option) {
    case 1:
    	moveControlMode = 0;
        if (myBSpline.nCtrlPts < 4) {
        	displayOption = 0;
		} else{
			myBSpline.computeBSplinePts();
			displayOption = 1;
		}
        break;
    case 2:
    	moveControlMode = 1;
    	myBSpline.computeBSplinePts();
		displayOption = 1;
        break;
    }
    glutPostRedisplay();
}

void reset() {
    displayOption = 0;
    myBSpline.reset();

    myBSpline.ctrlPts[myBSpline.nCtrlPts].x = startPos.first;
	myBSpline.ctrlPts[myBSpline.nCtrlPts].y = startPos.second;
	myBSpline.ctrlPts[myBSpline.nCtrlPts].z = 0;
	myBSpline.nCtrlPts++;
	myBSpline.nPts = (myBSpline.nCtrlPts - myBSpline.degree) * myBSpline.resolution;
	myBSpline.computeKnots();      // Update the knot vector
	myBSpline.computeBSplinePts(); // Recompute curve points
	glutPostRedisplay();

    glutIdleFunc(NULL);
}
