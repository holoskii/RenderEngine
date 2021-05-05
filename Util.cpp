#include "Util.h"

#include <fstream>
#include <iostream>
#include <strstream>
#include <cassert>

int polygon::clipAgainstPlane(vec4 plane_p, vec4 plane_n, polygon& in_tri, polygon& out_tri1, polygon& out_tri2)
{
	// Make sure plane normal is indeed normal
	plane_n = plane_n.normalize();

	// Return signed shortest distance from point to plane, plane normal must be normalised
	auto dist = [&](vec4& p)
	{
		vec4 n = p.normalize();
		return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - plane_n.dot(plane_p));
	};

	// Create two temporary storage arrays to classify points either side of plane
	// If distance sign is positive, point lies on "inside" of plane
	vec4* inside_points[3];  int nInsidePointCount = 0;
	vec4* outside_points[3]; int nOutsidePointCount = 0;

	// Get signed distance of each point in polygon to plane
	float d0 = dist(in_tri.p[0]);
	float d1 = dist(in_tri.p[1]);
	float d2 = dist(in_tri.p[2]);

	if (d0 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[0]; }
	else { outside_points[nOutsidePointCount++] = &in_tri.p[0]; }
	if (d1 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[1]; }
	else { outside_points[nOutsidePointCount++] = &in_tri.p[1]; }
	if (d2 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[2]; }
	else { outside_points[nOutsidePointCount++] = &in_tri.p[2]; }

	// Now classify polygon points, and break the input polygon into 
	// smaller output triangles if required. There are four possible
	// outcomes...

	if (nInsidePointCount == 0)
	{
		// All points lie on the outside of plane, so clip whole polygon
		// It ceases to exist

		return 0; // No returned triangles are valid
	}

	if (nInsidePointCount == 3)
	{
		// All points lie on the inside of plane, so do nothing
		// and allow the polygon to simply pass through
		out_tri1 = in_tri;

		return 1; // Just the one returned original polygon is valid
	}

	if (nInsidePointCount == 1 && nOutsidePointCount == 2)
	{
		// Triangle should be clipped. As two points lie outside
		// the plane, the polygon simply becomes a smaller polygon

		// Copy appearance info to new polygon
		out_tri1.color = in_tri.color;

		// The inside point is valid, so keep that...
		out_tri1.p[0] = *inside_points[0];

		// but the two new points are at the locations where the 
		// original sides of the polygon (lines) intersect with the plane
		out_tri1.p[1] = vec4::planeIntersect(plane_p, plane_n, *inside_points[0], *outside_points[0]);
		out_tri1.p[2] = vec4::planeIntersect(plane_p, plane_n, *inside_points[0], *outside_points[1]);

		return 1; // Return the newly formed single polygon
	}

	if (nInsidePointCount == 2 && nOutsidePointCount == 1)
	{
		// Triangle should be clipped. As two points lie inside the plane,
		// the clipped polygon becomes a "quad". Fortunately, we can
		// represent a quad with two new triangles

		// Copy appearance info to new triangles
		out_tri1.color = in_tri.color;

		out_tri2.color = in_tri.color;

		// The first polygon consists of the two inside points and a new
		// point determined by the location where one side of the polygon
		// intersects with the plane
		out_tri1.p[0] = *inside_points[0];
		out_tri1.p[1] = *inside_points[1];
		out_tri1.p[2] = vec4::planeIntersect(plane_p, plane_n, *inside_points[0], *outside_points[0]);

		// The second polygon is composed of one of he inside points, a
		// new point determined by the intersection of the other side of the 
		// polygon and the plane, and the newly created point above
		out_tri2.p[0] = *inside_points[1];
		out_tri2.p[1] = out_tri1.p[2];
		out_tri2.p[2] = vec4::planeIntersect(plane_p, plane_n, *inside_points[1], *outside_points[0]);

		return 2; // Return two newly formed triangles which form a quad
	}

	return 0;
}

bool mesh::loadObjectFile(std::string inFilename) {
	
	std::ifstream f(inFilename);
	if (!f.is_open()) return false;

	// Local cache of verts
	std::vector<vec4> verts;

	while (!f.eof())
	{
		std::string line;
		std::getline(f, line);

		if (line.length() < 3) {
			return true;
		}

		std::strstream s;
		s << line;

		std::string lineHeader;
		s >> lineHeader;
		if (s.fail()) {
			std::cout << "Reading failed: line header" << std::endl;
			return false;
		}

		if (lineHeader == "v") {
			vec4 v;
			s >> v.x >> v.y >> v.z;
			if (s.fail()) {
				std::cout << "Reading failed: vertex" << std::endl;
				return false;
			}
			verts.push_back(v);
		}
		else if (lineHeader == "f") {
			int count = std::count(line.begin(), line.end(), '/');

			if (count == 0) {
				int v[3];
				s >> v[0] >> v[1] >> v[2];
				if (s.fail()) {
					std::cout << "Reading failed: polygon" << std::endl;
					return false;
				}
				polygon poly;
				poly.p[0] = verts[v[0] - 1];
				poly.p[1] = verts[v[1] - 1];
				poly.p[2] = verts[v[2] - 1];

				this->tris.push_back(poly);
			}
			else if (count == 6 || count == 8) {
				char junkChar;
				int junkInt;

				int v[3];
				s >> v[0] >> junkChar >> junkInt >> junkChar >> junkInt >>
					v[1] >> junkChar >> junkInt >> junkChar >> junkInt >>
					v[2];
				if (s.fail()) {
					std::cout << "Reading failed: polygon" << std::endl;
					return false;
				}
				polygon poly;
				poly.p[0] = verts[v[0] - 1];
				poly.p[1] = verts[v[1] - 1];
				poly.p[2] = verts[v[2] - 1];

				this->tris.push_back(poly);
			} 
			else {
				std::cout << "Unknown obj file" << std::endl;
				return false;
			}
		}
	}
	return true;
	
	/*
	std::ifstream f(inFilename);
	if (!f.is_open()) return false;

	// Local cache of verts
	std::vector<vec4> verts;

	while (!f.eof())
	{
		char line[128];
		f.getline(line, 128);

		std::strstream s;
		s << line;

		char junk;

		if (line[0] == 'v')
		{
			vec4 v;
			s >> junk >> v.x >> v.y >> v.z;
			verts.push_back(v);
		}

		if (line[0] == 'f')
		{
			int f[3];
			s >> junk >> f[0] >> f[1] >> f[2];
			this->tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
		}
	}
	return true;*/
}
