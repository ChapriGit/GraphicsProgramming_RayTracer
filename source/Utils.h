#pragma once
#include <cassert>
#include <fstream>
#include <iostream>
#include "Math.h"
#include "DataTypes.h"
#include "TriangleMesh.h"

namespace dae
{
	namespace GeometryUtils
	{

#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			// B = dot(2*dir, (Pr -Ps))
			float b = Vector3::Dot(2 * ray.direction, ray.origin - sphere.origin);

			// C = |Pr - Ps|^2 - r^2
			float c = Vector3::Dot(ray.origin - sphere.origin, ray.origin - sphere.origin) - Square(sphere.radius);
 			
			float discriminant = b*b - 4 * c;
			bool hit = discriminant > 0;

			// If discriminant is smaller than 0, there is no hit with the sphere. Return.
			if (!hit)
				return false;

			// Find a t greater than 0, to make sure it is in front of the camera. Otherwise, return.
			float t = -b - sqrt(b*b - 4 * c);
			t = t / 2;

			if (t < ray.min || t > ray.max) {
				t = -b + sqrt(b*b - 4 * c);
				t = t / 2;
				if (t < ray.min || t > ray.max) {
					return false;
				}
			}

			if (ignoreHitRecord)
				return true;

			// Set the hitRecord if no t smaller was found yet.
			hitRecord.didHit = true;
			if (hitRecord.t > t) {
				hitRecord.t = t;
				hitRecord.materialIndex = sphere.materialIndex;
				hitRecord.origin = ray.origin + ray.direction * t;
				hitRecord.normal = (hitRecord.origin - sphere.origin)/sphere.radius;
			}

			return true;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			// t = dot((Oplane - Oray), n) / dot(dir, n)

			float t = Vector3::Dot((plane.origin - ray.origin), plane.normal);
			t = t / Vector3::Dot(ray.direction, plane.normal);

			if (t > ray.min && t < ray.max) {
				if (ignoreHitRecord)
					return true;

				if (hitRecord.t > t) {
					hitRecord.didHit = true;
					hitRecord.t = t;
					hitRecord.materialIndex = plane.materialIndex;
					hitRecord.origin = ray.origin + ray.direction * t;
					hitRecord.normal = plane.normal.Normalized();

					return true;
				}
			}
			
			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			switch (triangle.cullMode) {
			case (TriangleCullMode::FrontFaceCulling):
				if (Vector3::Dot(triangle.normal, -ray.direction) > -0.001)
					return false;
				break;
			case (TriangleCullMode::BackFaceCulling):
				if (Vector3::Dot(triangle.normal, -ray.direction) < 0.001)
					return false;
				break;
			case (TriangleCullMode::NoCulling):
				float dotNDir = Vector3::Dot(triangle.normal, -ray.direction);
				if (dotNDir < 0.001 && dotNDir > -0.001)
					return false;
				break;
			}

			// MÖLLER-TRUMBORE
			/* Solution of the system [D (v1 - v0) (v2 - v0)] [t u v] = O - v0
			*			with v0, v1, v2 the corners of the triangle and u, v and w (= 1-u-v) the barycentric coordinates
			*			of the intersection between the triangle plane and the direction of the ray D with origin O
			* 
			*	=> Cramer's rule to find the solution: Ax = b => xi = det(Ai) / det(A)
			*			Where Ai is the matrix formed by replacing the ith column of A by b
			* 
			*			A is a 3x3 matrix
			* 
			*			=> b = O - v0
			*				=>	A1 = [b, (v1 - v0), (v2 - v0)], A2 = [D, b, (v2 - v0)], A3 = [D, (v1 - v0), b]
			*					t = det(A1)/det(A), u = det(A2)/det(A), v = det(A3)/det(A)
			* 
			*	0 < u, v, w < 1 to hit the triangle
			*/

			Vector3 b = ray.origin - triangle.v0;
			Vector3 e10 = triangle.v1 - triangle.v0;
			Vector3 e20 = triangle.v2 - triangle.v0;

			/* Determinant of a 3x3: Pick any row or column:
			*		Sum(el * cofactor(el))
			*			=> dx * cof(e10.y e20.y, e10.z e20.z) + dy * cof(e10.x e20.x, e10.z e20.z) 
			*				+ dz * cof(e10.x e20.x, e10.y e20.y)
			*		For A1 same thing, but with b instead of d
			* 
			*		cof(a b, c d) = a*d - b*c
			* 
			*		Aka cross and dot product :P
			*/
		
			Vector3 cofE = Vector3::Cross(e10, e20);
			float detA = Vector3::Dot(-ray.direction, cofE);
			
			float detA1 = Vector3::Dot(b, cofE);
			float t = (float) detA1 / detA;

			if (t < ray.min || t > ray.max || hitRecord.t < t)
				return false;

			Vector3 cof_B_E20 = Vector3::Cross(b, e20);
			float detA2 = Vector3::Dot(-ray.direction, cof_B_E20);
			float u = (float) detA2 / detA;

			if (u < 0 || u > 1)
				return false;

			Vector3 cof_E10_B = Vector3::Cross(e10, b);
			float detA3 = Vector3::Dot(-ray.direction, cof_E10_B);
			float v = (float) detA3 / detA;

			// u is already between 0 and 1, v has to be below 1, but w also has to be above 0: w = 1 - u - v > 0
			if (v < 0 || v + u > 1)
				return false;

			if (ignoreHitRecord)
				return true;

			hitRecord.t = t;
			hitRecord.didHit = true;
			hitRecord.materialIndex = triangle.materialIndex;
			hitRecord.normal = triangle.normal;
			hitRecord.origin = ray.origin + t * ray.direction;

			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			if (!mesh.SlabTest_TriangleMesh(ray)) {
				return false;
			}

			const Transformation finalTransform = mesh.scaleTransform.append(mesh.rotationTransform.append(mesh.translationTransform));
			Ray invertedRay = finalTransform.inverseTransformRay(ray);

			std::vector<int> trianglesToTest = mesh.traverseTree(0, invertedRay);

			for (int i : trianglesToTest) {
				int i0 = mesh.indices[i];
				int i1 = mesh.indices[i + 1];
				int i2 = mesh.indices[i + 2];

				Vector3 v0 = mesh.transformedPositions[i0];
				Vector3 v1 = mesh.transformedPositions[i1];
				Vector3 v2 = mesh.transformedPositions[i2];

				Triangle triangle = Triangle(v0, v1, v2, mesh.transformedNormals[i]);
				triangle.cullMode = mesh.cullMode;
				if (GeometryUtils::HitTest_Triangle(triangle, ray, hitRecord, ignoreHitRecord)) {
					if (ignoreHitRecord)
						return true;
					hitRecord.materialIndex = mesh.materialIndex;
				}
			}

			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}