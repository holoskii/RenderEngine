#pragma once

#include "Util.h"

#include "SFML/Graphics.hpp"

class RenderEngine
{
public:
	int width = 1920;
	int height = 1080;
	sf::RenderWindow* window;

	

	mesh object;
	mat4x4 matProj;					// Matrix that converts from view space to screen space
	vec4 vCamera = { -5, 1, 0 };	// Location of camera in world space
	vec4 vLookDir = { 0, 0, 0 };	// Direction vector along the direction camera points
	float fYaw = -45;					// FPS Camera rotation in XZ plane
	float fTheta = 0;				// Spins World transform
	float frameTime = 0;

	RenderEngine();
	void run();
	void render(float fElapsedTime);
	void drawBlankTriange(polygon& poly);
	void drawColorTriange(polygon poly);
};



