#pragma once

#include "Util.h"

#include "SFML/Graphics.hpp"

class RenderEngine
{
public:
	// because of OpenGL render inside window 
	// width and height does not have much effect on performance
	int windowWidth = 1920;
	int windowHeight = 1080;
	

	sf::RenderWindow window;		// widnow handle
	mesh objectMesh;				// object to be rendered
	mat4x4 matProj;					// world -> camere view projection	
	vec4 vCamera = { -5, 1, 0 };	// camera location
	vec4 vLookDir;					// camera direction
	float fYaw = -45;				// camera rotation in horizontal plane
	float fTheta = 0;				// object rotation
	float frameTime = 0;			// time between frames
	
	int maxFrameRate = 0;			// limit framerate

	// create window with default size
	RenderEngine();

	// start render
	void run();

	// render window content
	void render(float fElapsedTime);

	// draw triangle with SFML as 3 thin lines
	void drawBlankTriange(polygon& poly);

	// draw solid triangle with SFML, colored as in poly.color RGBA format
	void drawColorTriange(polygon& poly);
};



