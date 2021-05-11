/* Created: 29.04.2021
 * Author: Makar Ivashko
 * Short description: 3d rendering engine,
 * projects polygons from 3d space into 2d screen
 */

#include "RenderEngine.h"

#include <iostream>
#include <algorithm>
#include <chrono>
#include <list>

RenderEngine::RenderEngine() {
	// add minimum antialiasing
	sf::ContextSettings settings;
	settings.antialiasingLevel = 2;

	window.create(sf::VideoMode(windowWidth, windowHeight), "SFML shapes", sf::Style::Default, settings);
}

void RenderEngine::run(const std::string& filename, const bool toRotate) {
	// load object file
	if (!objectMesh.loadObjectFile(filename)) {
		std::cout << "Error openning file " << std::endl;
		exit(1);
	}

	// fill projection matrix
	matProj = mat4x4::createProjection(90.0f, (float)windowHeight / (float)windowWidth, 0.1f, 1000.0f);

	// frame time measurment
	auto start_time = std::chrono::high_resolution_clock::now();

	while (window.isOpen()) {

		// catch events
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			else if (event.type == sf::Event::Resized) {
				float aspectRatio = (float)windowHeight / (float)windowWidth;
				sf::Vector2u newSize = { event.size.width, (unsigned int)(event.size.width * aspectRatio) };
				window.setSize(newSize);
			}
		}

		// handle movement
		float moveMult = std::min(1.0f, 0.010f * frameTime);
		float rotationMult = std::min(0.7f, 0.0007f * frameTime);

		vec4 vForward = vLookDir * moveMult;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			vCamera = vCamera + vForward;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			vCamera = vCamera - vForward;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
			vCamera.y -= moveMult;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
			vCamera.y += moveMult;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			fYaw -= rotationMult;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			fYaw += rotationMult;
		}

		// rotate object
		if (toRotate) {
			fTheta += 2.5e-4f * frameTime;
		}

		// clear -> render -> display routine
		window.clear();
		render(frameTime);
		window.display();

		// frame time measurment
		auto elapsed = std::chrono::high_resolution_clock::now() - start_time;
		float renderTime = (std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count()) / 1000.0f;
		window.setTitle(std::string("Frametime: ").append(std::to_string(frameTime)));

		// limit framerate to 30
		if (maxFrameRate > 0) {
			int sleepTime = 1000.0f / maxFrameRate - (int)renderTime;
			if (sleepTime < 0) sleepTime = 0;
			sf::sleep(sf::milliseconds(sleepTime));
		}
		

		auto elapsed_time = std::chrono::high_resolution_clock::now() - start_time;
		frameTime = (std::chrono::duration_cast<std::chrono::microseconds>(elapsed_time).count()) / 1000.0f;
		start_time = std::chrono::high_resolution_clock::now();
	}
}

void RenderEngine::render(float fElapsedTime) {
	// create world tranform matrix
	mat4x4 matRotY = mat4x4::makeRotationY(fTheta);					// world rotation, better for object exhibition	
	mat4x4 matTrans = mat4x4::makeTranslation(0.0f, 0.0f, 5.0f);	// world translation (so camera won't stuck in smaller objects)
	mat4x4 matWorld = matRotY * matTrans; 

	// tranformation matrix for camera
	vec4 vUp = { 0, 1, 0 };
	vec4 vTarget = { 0, 0, 1 };

	vLookDir = mat4x4::makeRotationY(fYaw) * vTarget;
	vTarget = vCamera + vLookDir;
	mat4x4 matCamera = mat4x4::cameraTransform(vCamera, vTarget, vUp);

	// create view tranformation
	mat4x4 matView = matCamera.quickInverse();

	// vector containing all polygons transformed polygons
	std::vector<polygon> vecPolysToRaster;
	vecPolysToRaster.reserve(objectMesh.polys.size());

	// tranform polygons
	for (polygon& poly : objectMesh.polys) {
		polygon polyProjected, polyTransformed, polyViewed;

		// world transform
		polyTransformed.p[0] = matWorld * poly.p[0];
		polyTransformed.p[1] = matWorld * poly.p[1];
		polyTransformed.p[2] = matWorld * poly.p[2];

		// calculate normal as cross product of 2 polygon sides
		vec4 vNormal, line1, line2;
		line1 = polyTransformed.p[1] - polyTransformed.p[0];
		line2 = polyTransformed.p[2] - polyTransformed.p[0];
		vNormal = line1.cross(line2).normalize();

		// get camera rey to calculate illumination
		vec4 vCameraRay = polyTransformed.p[0] - vCamera;

		// if correct polygon side is visible
		// if (vNormal.dot(vCameraRay) < 0.0f) {}
		// illuminate
		vec4 light_direction = vec4(0.0f, 1.0f, -1.0f).normalize();

		// calculate illumination by angle between normal and light direction
		// color is shade of gray, so keep in from 0 to 255
		unsigned char colorInt;
		colorInt = std::max(0.05f, light_direction.dot(vNormal)) * 255.0f; 
		polyTransformed.color = (colorInt << 24) + (colorInt << 16) + (colorInt << 8);

		// tranform from world into view
		polyViewed.p[0] = matView * polyTransformed.p[0];
		polyViewed.p[1] = matView * polyTransformed.p[1];
		polyViewed.p[2] = matView * polyTransformed.p[2];
		polyViewed.color = polyTransformed.color;

		// polygon clipping agains camera plane
		int nClippedPolygons = 0;
		polygon clipped[2];
		nClippedPolygons = polygon::clipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, polyViewed, clipped[0], clipped[1]);

		// project results of clipping
		for (int n = 0; n < nClippedPolygons; n++) {
			// from view to screen (3D -> 2D)
			polyProjected.p[0] = matProj * clipped[n].p[0];
			polyProjected.p[1] = matProj * clipped[n].p[1];
			polyProjected.p[2] = matProj * clipped[n].p[2];
			polyProjected.color = clipped[n].color;

			// scaled them into view
			polyProjected.p[0] = polyProjected.p[0] / polyProjected.p[0].w;
			polyProjected.p[1] = polyProjected.p[1] / polyProjected.p[1].w;
			polyProjected.p[2] = polyProjected.p[2] / polyProjected.p[2].w;

			// invert x and y back
			polyProjected.p[0].x *= -1.0f;
			polyProjected.p[1].x *= -1.0f;
			polyProjected.p[2].x *= -1.0f;
			polyProjected.p[0].y *= -1.0f;
			polyProjected.p[1].y *= -1.0f;
			polyProjected.p[2].y *= -1.0f;

			// offset polygon into visible space
			vec4 vOffsetView = { 1, 1, 0 };
			polyProjected.p[0] = polyProjected.p[0] + vOffsetView;
			polyProjected.p[1] = polyProjected.p[1] + vOffsetView;
			polyProjected.p[2] = polyProjected.p[2] + vOffsetView;
			polyProjected.p[0].x *= 0.5f * windowWidth;
			polyProjected.p[0].y *= 0.5f * windowHeight;
			polyProjected.p[1].x *= 0.5f * windowWidth;
			polyProjected.p[1].y *= 0.5f * windowHeight;
			polyProjected.p[2].x *= 0.5f * windowWidth;
			polyProjected.p[2].y *= 0.5f * windowHeight;

			// push polygons in vector for further sorting
			vecPolysToRaster.push_back(polyProjected);
		}
		// }
	}

	// sort back to front
	sort(vecPolysToRaster.begin(), vecPolysToRaster.end(), [](polygon& t1, polygon& t2)
		{
			float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
			float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
			return z1 > z2;
		} );

	// polygon clipping against screen borders
	for (auto& polyToRaster : vecPolysToRaster) {
		// clipping agains screen edges may result in a lot of new polygons
		// so we'll create list to store them
		polygon clipped[2];
		std::list<polygon> listPolygons;

		// add original polygon
		listPolygons.push_back(polyToRaster);
		int nNewPolys = 1;

		for (int p = 0; p < 4; p++) {
			int nPolysToAdd = 0;
			while (nNewPolys > 0) {
				// take poly from list
				polygon test = listPolygons.front();
				listPolygons.pop_front();
				nNewPolys--;

				// clip against one of four screen edges
				switch (p)
				{
				case 0:	nPolysToAdd = polygon::clipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				case 1:	nPolysToAdd = polygon::clipAgainstPlane({ 0.0f, (float)windowHeight - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				case 2:	nPolysToAdd = polygon::clipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				case 3:	nPolysToAdd = polygon::clipAgainstPlane({ (float)windowWidth - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				}

				// add newly created polygons
				for (int w = 0; w < nPolysToAdd; w++) {
					listPolygons.push_back(clipped[w]);
				}
			}
			nNewPolys = (int)listPolygons.size();
		}


		// final draw of polygon
		for (auto& t : listPolygons) {
			drawColorTriange(t);
			// drawBlankTriange(t);
		}
	}
}

void RenderEngine::drawBlankTriange(polygon& poly) {
	sf::Vertex line[2];

	line[0] = sf::Vertex(sf::Vector2f(poly.p[0].x, poly.p[0].y));
	line[1] = sf::Vertex(sf::Vector2f(poly.p[1].x, poly.p[1].y));
	window.draw(line, 2, sf::Lines);

	line[0] = sf::Vertex(sf::Vector2f(poly.p[1].x, poly.p[1].y));
	line[1] = sf::Vertex(sf::Vector2f(poly.p[2].x, poly.p[2].y));
	window.draw(line, 2, sf::Lines);

	line[0] = sf::Vertex(sf::Vector2f(poly.p[2].x, poly.p[2].y));
	line[1] = sf::Vertex(sf::Vector2f(poly.p[0].x, poly.p[0].y));
	window.draw(line, 2, sf::Lines);
}

void RenderEngine::drawColorTriange(polygon& poly) {
	sf::ConvexShape shape;

	shape.setPointCount(3);

	shape.setPoint(0, sf::Vector2f(poly.p[0].x, poly.p[0].y));
	shape.setPoint(1, sf::Vector2f(poly.p[1].x, poly.p[1].y));
	shape.setPoint(2, sf::Vector2f(poly.p[2].x, poly.p[2].y));

	shape.setFillColor(sf::Color(poly.color | 0x000000FF));

	window.draw(shape);
}