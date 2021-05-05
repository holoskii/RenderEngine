#include "RenderEngine.h"

#include <iostream>
#include <algorithm>
#include <chrono>
#include <list>
using namespace std;

RenderEngine::RenderEngine()
{
	sf::ContextSettings settings;
	settings.antialiasingLevel = 2;

	window = new sf::RenderWindow(sf::VideoMode(width, height), "SFML shapes", sf::Style::Default, settings);
}

void RenderEngine::run()
{
	// Load object file
	if (!object.loadObjectFile("sphere.obj")) {
		std::cout << "Error openning file " << std::endl;
		exit(1);
	}

	// Projection Matrix
	matProj = mat4x4::createProjection(90.0f, (float)height / (float)width, 0.1f, 1000.0f);

	auto start_time = std::chrono::high_resolution_clock::now();

	while (window->isOpen()) {

		sf::Event event;
		while (window->pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window->close();
			}
		}

		float moveMult = std::min(1.0f, 0.010f * frameTime);
		float rotationMult = std::min(0.7f, 0.0007f * frameTime);

		vec4 vForward = vLookDir * moveMult;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			vCamera = vCamera + vForward;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			vCamera = vCamera - vForward;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			fYaw -= rotationMult;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			fYaw += rotationMult;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
			vCamera.y -= moveMult;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
			vCamera.y += moveMult;
		}

		window->clear();
		render(frameTime);
		window->display();

		auto elapsed = std::chrono::high_resolution_clock::now() - start_time;
		float renderTime = (std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count()) / 1000.0f;
		window->setTitle(std::string("Frametime: ").append(std::to_string(frameTime)));

		int sleepTime = 33 - (int)renderTime;
		if (sleepTime < 0) sleepTime = 0;
		sf::sleep(sf::milliseconds(sleepTime));

		auto elapsed_time = std::chrono::high_resolution_clock::now() - start_time;
		frameTime = (std::chrono::duration_cast<std::chrono::microseconds>(elapsed_time).count()) / 1000.0f;

		start_time = std::chrono::high_resolution_clock::now();
	}

	// return true;
}

void RenderEngine::render(float fElapsedTime)
{
	// Set up "World Tranmsform" though not updating theta 
	// makes this a bit redundant
	mat4x4 matRotZ, matRotX;
	//fTheta += 1.0f * fElapsedTime; // Uncomment to spin me right round baby right round
	matRotZ = mat4x4::makeRotationZ(fTheta * 0.5f);
	matRotX = mat4x4::makeRotationX(fTheta);

	mat4x4 matTrans = mat4x4::makeTranslation(0.0f, 0.0f, 5.0f);

	mat4x4 matWorld;
	matWorld = matRotZ * matRotX; // Transform by rotation
	matWorld = matWorld * matTrans; // Transform by translation

	// Create "Point At" Matrix for camera
	vec4 vUp = { 0,1,0 };
	vec4 vTarget = { 0,0,1 };
	mat4x4 matCameraRot = mat4x4::makeRotationY(fYaw);
	vLookDir = matCameraRot * vTarget;
	vTarget = vCamera + vLookDir;
	mat4x4 matCamera = mat4x4::cameraTransform(vCamera, vTarget, vUp);

	// Make view matrix from camera
	mat4x4 matView = matCamera.quickInverse();

	// Store triagles for rastering later
	vector<polygon> vecTrianglesToRaster;

	// Draw Triangles
	for (auto poly : object.tris)
	{
		polygon triProjected, triTransformed, triViewed;

		// World Matrix Transform
		triTransformed.p[0] = matWorld * poly.p[0];
		triTransformed.p[1] = matWorld * poly.p[1];
		triTransformed.p[2] = matWorld * poly.p[2];

		// Calculate polygon Normal
		vec4 normal, line1, line2;

		// Get lines either side of polygon
		line1 = triTransformed.p[1] - triTransformed.p[0];
		line2 = triTransformed.p[2] - triTransformed.p[0];

		// Take cross product of lines to get normal to polygon surface
		normal = line1.cross(line2);

		// You normally need to normalise a normal!
		normal = normal.normalize();

		// Get Ray from polygon to camera
		vec4 vCameraRay = triTransformed.p[0] - vCamera;

		// If ray is aligned with normal, then polygon is visible
		if (normal.dot(vCameraRay) < 0.0f)
		{
			// Illumination
			vec4 light_direction = { 0.0f, 1.0f, -1.0f };
			light_direction = light_direction.normalize();

			// How "aligned" are light direction and polygon surface normal?
			float dp = max(0.1f, light_direction.dot(normal));
			uint8_t colorInt = (uint8_t)(dp * 255.0f);
			// triViewed.color = colorInt << 24 + colorInt << 16 + colorInt << 8;

			// Choose console colours as required (much easier with RGB)
			// CHAR_INFO c = GetColour(dp);
			// triTransformed.color = 0x777777FF;
			// triTransformed.sym = c.Char.UnicodeChar;
			triTransformed.color = (colorInt << 24) + (colorInt << 16) + (colorInt << 8);


			// Convert World Space --> View Space
			triViewed.p[0] = matView * triTransformed.p[0];
			triViewed.p[1] = matView * triTransformed.p[1];
			triViewed.p[2] = matView * triTransformed.p[2];
			triViewed.color = triTransformed.color;

			// Clip Viewed Triangle against near plane, this could form two additional
			// additional triangles. 
			int nClippedTriangles = 0;
			polygon clipped[2];
			nClippedTriangles = polygon::clipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);

			// We may end up with multiple triangles form the clip, so project as
			// required
			for (int n = 0; n < nClippedTriangles; n++)
			{
				// Project triangles from 3D --> 2D
				triProjected.p[0] = matProj * clipped[n].p[0];
				triProjected.p[1] = matProj * clipped[n].p[1];
				triProjected.p[2] = matProj * clipped[n].p[2];
				triProjected.color = clipped[n].color;

				// Scale into view, we moved the normalising into cartesian space
				// out of the matrix.vector function from the previous videos, so
				// do this manually
				triProjected.p[0] = triProjected.p[0] / triProjected.p[0].w;
				triProjected.p[1] = triProjected.p[1] / triProjected.p[1].w;
				triProjected.p[2] = triProjected.p[2] / triProjected.p[2].w;

				// X/Y are inverted so put them back
				triProjected.p[0].x *= -1.0f;
				triProjected.p[1].x *= -1.0f;
				triProjected.p[2].x *= -1.0f;
				triProjected.p[0].y *= -1.0f;
				triProjected.p[1].y *= -1.0f;
				triProjected.p[2].y *= -1.0f;

				// Offset verts into visible normalised space
				vec4 vOffsetView = { 1, 1, 0 };
				triProjected.p[0] = triProjected.p[0] + vOffsetView;
				triProjected.p[1] = triProjected.p[1] + vOffsetView;
				triProjected.p[2] = triProjected.p[2] + vOffsetView;
				triProjected.p[0].x *= 0.5f * (float)width;
				triProjected.p[0].y *= 0.5f * (float)height;
				triProjected.p[1].x *= 0.5f * (float)width;
				triProjected.p[1].y *= 0.5f * (float)height;
				triProjected.p[2].x *= 0.5f * (float)width;
				triProjected.p[2].y *= 0.5f * (float)height;

				// Store polygon for sorting
				vecTrianglesToRaster.push_back(triProjected);
			}
		}
	}

	// Sort triangles from back to front
	sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](polygon& t1, polygon& t2)
		{
			float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
			float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
			return z1 > z2;
		}
	);

	// Loop through all transformed, viewed, projected, and sorted triangles
	for (auto& triToRaster : vecTrianglesToRaster)
	{
		// Clip triangles against all four screen edges, this could yield
		// a bunch of triangles, so create a queue that we traverse to 
		//  ensure we only test new triangles generated against planes
		polygon clipped[2];
		list<polygon> listTriangles;

		// Add initial polygon
		listTriangles.push_back(triToRaster);
		int nNewTriangles = 1;

		for (int p = 0; p < 4; p++)
		{
			int nTrisToAdd = 0;
			while (nNewTriangles > 0)
			{
				// Take polygon from front of queue
				polygon test = listTriangles.front();
				listTriangles.pop_front();
				nNewTriangles--;

				// Clip it against a plane. We only need to test each 
				// subsequent plane, against subsequent new triangles
				// as all triangles after a plane clip are guaranteed
				// to lie on the inside of the plane. I like how this
				// comment is almost completely and utterly justified
				switch (p)
				{
				case 0:	nTrisToAdd = polygon::clipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				case 1:	nTrisToAdd = polygon::clipAgainstPlane({ 0.0f, (float)height - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				case 2:	nTrisToAdd = polygon::clipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				case 3:	nTrisToAdd = polygon::clipAgainstPlane({ (float)width - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				}

				// Clipping may yield a variable number of triangles, so
				// add these new ones to the back of the queue for subsequent
				// clipping against next planes
				for (int w = 0; w < nTrisToAdd; w++)
					listTriangles.push_back(clipped[w]);
			}
			nNewTriangles = (int)listTriangles.size();
		}


		// Draw the transformed, viewed, clipped, projected, sorted, clipped triangles
		for (auto& t : listTriangles)
		{

			drawColorTriange(t);
			// drawBlankTriange(t);
			// DrawTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, PIXEL_SOLID, FG_BLACK);
		}
	}
}

void RenderEngine::drawBlankTriange(polygon& poly) {
	sf::Vertex line[2];

	line[0] = sf::Vertex(sf::Vector2f(poly.p[0].x, poly.p[0].y));
	line[1] = sf::Vertex(sf::Vector2f(poly.p[1].x, poly.p[1].y));
	window->draw(line, 2, sf::Lines);

	line[0] = sf::Vertex(sf::Vector2f(poly.p[1].x, poly.p[1].y));
	line[1] = sf::Vertex(sf::Vector2f(poly.p[2].x, poly.p[2].y));
	window->draw(line, 2, sf::Lines);

	line[0] = sf::Vertex(sf::Vector2f(poly.p[2].x, poly.p[2].y));
	line[1] = sf::Vertex(sf::Vector2f(poly.p[0].x, poly.p[0].y));
	window->draw(line, 2, sf::Lines);
}

void RenderEngine::drawColorTriange(polygon poly) {


	// create an empty shape
	sf::ConvexShape shape;

	// resize it to 5 points
	shape.setPointCount(3);

	// define the points
	shape.setPoint(0, sf::Vector2f(poly.p[0].x, poly.p[0].y));
	shape.setPoint(1, sf::Vector2f(poly.p[1].x, poly.p[1].y));
	shape.setPoint(2, sf::Vector2f(poly.p[2].x, poly.p[2].y));

	shape.setFillColor(sf::Color(poly.color | 0x000000FF));

	window->draw(shape);
}