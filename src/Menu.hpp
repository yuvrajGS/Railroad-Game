/*
 *  Author: Yuvraj Singh
 *  Version: 2024-11-20
 */
#ifndef MENU_HPP_
#define MENU_HPP_

#include "glsl/Angel.h"
#include <GL/glut.h>

void menu();
void mainMenu(GLint option);
void ObjSubMenu(GLint objectOption);
void MCSTransMenu(GLint transOption);
void WCSTransMenu(GLint transOption);
void VCSTransMenu(GLint transOption);
void MCSTransform(GLint);
void WCSTransform(GLint);
void VCSTransform(GLint);

void cullMenu(GLint option);
void lightMenu(GLint option);
void lightTransform(GLint);
void shadeMenu(GLint option);
void animateMenu(GLint option);

void curveSurfaceMenu(GLint option);
void move();
void reset();
void run();
void newTerrain();

#endif
