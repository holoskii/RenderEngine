/* Created: 29.04.2021
 * Author: Makar Ivashko
 * Short description: polygon and mesh classes, with some utility functions
 */

#include "Util.h"

#include <fstream>
#include <iostream>
#include <strstream>
#include <cassert>

int polygon::clipAgainstPlane(vec4 plane_p, vec4 plane_n, polygon& in_poly, polygon& out_poly1, polygon& out_poly2) {
	// normilize plane
	plane_n = plane_n.normalize();

	// distance from plane to point
	// If distance is negative point is "outside" the plane
	auto dist = [&](vec4& p) {
		vec4 n = p.normalize();
		return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - plane_n.dot(plane_p));
	};

	// poins on both sizes of plane
	vec4* inside_points[3];  int nInsidePointCount = 0;
	vec4* outside_points[3]; int nOutsidePointCount = 0;

	// get distance
	float d0 = dist(in_poly.p[0]);
	float d1 = dist(in_poly.p[1]);
	float d2 = dist(in_poly.p[2]);

	if (d0 >= 0)	
		inside_points[nInsidePointCount++]	= &in_poly.p[0];
	else
		outside_points[nOutsidePointCount++]= &in_poly.p[0];
	if (d1 >= 0)
		inside_points[nInsidePointCount++] = &in_poly.p[1];
	else 
		outside_points[nOutsidePointCount++] = &in_poly.p[1];
	if (d2 >= 0)
		inside_points[nInsidePointCount++] = &in_poly.p[2];
	else
		outside_points[nOutsidePointCount++] = &in_poly.p[2];

	// based on points layout output triangle
	if (nInsidePointCount == 0) {
		// triangle is out of plane, so it is invalid
		return 0;
	}

	if (nInsidePointCount == 3) {
		// original polygon is valid
		out_poly1 = in_poly;
		return 1;
	}

	if (nInsidePointCount == 1 && nOutsidePointCount == 2) {
		// polygon gets clipped into smaller polygon
		out_poly1.color = in_poly.color;
		out_poly1.p[0] = *inside_points[0];
		out_poly1.p[1] = vec4::planeIntersect(plane_p, plane_n, *inside_points[0], *outside_points[0]);
		out_poly1.p[2] = vec4::planeIntersect(plane_p, plane_n, *inside_points[0], *outside_points[1]);
		return 1;
	}

	if (nInsidePointCount == 2 && nOutsidePointCount == 1) {
		// polygon gets clipped into rectangle => will be 2 output polygons
		out_poly1.color = in_poly.color;
		out_poly2.color = in_poly.color;

		out_poly1.p[0] = *inside_points[0];
		out_poly1.p[1] = *inside_points[1];
		out_poly1.p[2] = vec4::planeIntersect(plane_p, plane_n, *inside_points[0], *outside_points[0]);

		out_poly2.p[0] = *inside_points[1];
		out_poly2.p[1] = out_poly1.p[2];
		out_poly2.p[2] = vec4::planeIntersect(plane_p, plane_n, *inside_points[1], *outside_points[0]);
		return 2;
	}

	return 0;
}

bool mesh::loadObjectFile(std::string inFilename) {
	std::ifstream f(inFilename);
	if (!f.is_open()) return false;

	std::vector<vec4> verts;

	while (!f.eof())
	{
		std::string line;
		std::getline(f, line);

		if (line.length() < 3) {
			continue;
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

				this->polys.push_back(poly);
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

				this->polys.push_back(poly);
			} 
			else {
				std::cout << "Unknown obj file" << std::endl;
				return false;
			}
		}
		else {
			// std::cout << "Unknown line header " << lineHeader << std::endl;
		}
	}
	return true;

}
