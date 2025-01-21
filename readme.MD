# 2D Railroad Game Documentation

## **1. Overview**

### **Objective**

The goal of the game is to construct a railroad track path from the starting station to the goal station, ensuring that the rail reaches the destination successfully.

### **Gameplay**

Gameplay revolves around defining the railroad path using a rational B-spline curve. Players can move and adjust control points to shape the curve, with a maximum of 10 control points available. Each control point’s weight can also be altered, allowing for dynamic adjustments. The B-spline curve ensures a smooth path for the train, which must avoid obstacles such as rocks and water scattered across the terrain.

### **Key Features**

- **B-Spline Curve**: Provides a smooth, adjustable path for the railroad.
- **Procedural Terrain Generation**: Uses Perlin noise for dynamic map creation.
- **Train Animation**: Simulates the train moving along the generated track.
- **Dynamic Curve Control**: Allows users to interactively place, move, and adjust control points.

---

## **2. Installation and Setup**

### **Dependencies**

The following libraries are required:

- `<GL/glut.h>`
- `<GL/glew.h>`
- `"GL/glaux.h"`
- `<GL/freeglut.h>`
- `<GL/freeglut_ext.h>`

### **Compilation Instructions**

The primary entry point is `main.cpp`. Ensure all dependent libraries are installed and link them appropriately during compilation.

### **Configuration Options**

Adjustable parameters include:

- **Window Dimensions**: Customize the width and height.
- **Train Parameters**: Speed and size of the train.
- **Terrain Generation**: Grid size for Perlin noise, terrain resolution, scale and obstacle generation.
- **Control Points**: Number of control points (minimum 4, maximum 10).
- **Path Parameters**: B-spline degree and resolution (points per segment).
- **Scoring System**: Adjust scoring weights for distance, smoothness, and control point count.

---

## **3. Gameplay Mechanics**

### **Controls**

- **Right-Click Menu Options**:
  - Reset the B-spline: Clears the track for a fresh start.
  - Define Track: Place and adjust control points.
    - **Control Point Adjustment**:
      - Drag control points to new positions.
      - Press `W` to increase weight, `S` to decrease weight, and `R` to reset weight.
  - Start Train: Initiates the train animation along the generated track.
  - Generate New Terrain: Refreshes the terrain with new obstacles.
  - Quit: Exits the game.

### **Rules**

- The starting control point must be located at the start station, and the last control point at the goal station.
- A minimum of 4 and a maximum of 10 control points are allowed.
- The track must avoid all collisions with obstacles.

### **Scoring System**

The final score is calculated as follows:

1. **Distance Bonus**: Based on the distance between the start and end stations.
2. **Control Point Bonus**: Based on the number of control points used.
3. **Smoothness Bonus**: Based on curve smoothness, calculated using a custom algorithm.
4. **Total Score**: Sum of the above bonuses.

---

## **4. Architecture Overview**

### **File Descriptions**

- **`TerrainGenerator`**: Handles terrain generation using Perlin noise. Manages terrain textures, obstacle placement, and ensures valid start and end positions.
- **`BSpline`**:
  - Manages curve generation, rendering, and train animation.
  - Implements collision detection to ensure track validity.
  - Inherits from the parent `Curve` class.
- **`Curve`**: Base class for curve-related operations.
- **`RGBpixmap`**: Handles texture loading for terrain and train elements.
- **`main.cpp`**:
  - Initializes the game, window, and OpenGL settings.
  - Handles display and gameplay logic integration.
- **`Menu`**: Implements the right-click menu functionality, allowing players to interact with game options.
