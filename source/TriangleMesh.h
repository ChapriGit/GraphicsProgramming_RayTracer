#pragma once
#include "Math.h"
#include "vector"
#include "DataTypes.h"
#include "Transformation.h"

namespace dae
{
	struct TriangleMesh
	{
		TriangleMesh() = default;
		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, TriangleCullMode _cullMode) :
			positions(_positions), indices(_indices), cullMode(_cullMode)
		{
			//Calculate Normals
			CalculateNormals();

			//Update Transforms
			UpdateTransforms();
		}

		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, const std::vector<Vector3>& _normals, TriangleCullMode _cullMode) :
			positions(_positions), indices(_indices), normals(_normals), cullMode(_cullMode)
		{
			UpdateTransforms();
		}

		std::vector<Vector3> positions{};
		std::vector<Vector3> normals{};
		std::vector<int> indices{};
		unsigned char materialIndex{};

		TriangleCullMode cullMode{ TriangleCullMode::BackFaceCulling };

		Transformation rotationTransform{};
		Transformation translationTransform{};
		Transformation scaleTransform{};

		std::vector<Vector3> transformedPositions{};
		std::vector<Vector3> transformedNormals{};

		Vector3 minAABB{};
		Vector3 maxAABB{};


		void Translate(const Vector3& translation)
		{
			translationTransform = Transformation::translate(translation.x, translation.y, translation.z);
		}

		void RotateY(float yaw)
		{
			rotationTransform = Transformation::rotateY(yaw);
		}

		void Scale(const Vector3& scale)
		{
			scaleTransform = Transformation::scale(scale.x, scale.y, scale.z);
		}

		void AppendTriangle(const Triangle& triangle, bool ignoreTransformUpdate = false)
		{
			int startIndex = static_cast<int>(positions.size());

			positions.push_back(triangle.v0);
			positions.push_back(triangle.v1);
			positions.push_back(triangle.v2);

			indices.push_back(startIndex);
			indices.push_back(++startIndex);
			indices.push_back(++startIndex);

			normals.push_back(triangle.normal);

			//Not ideal, but making sure all vertices are updated
			if (!ignoreTransformUpdate)
				UpdateTransforms();
		}

		void CalculateNormals()
		{
			normals.reserve(positions.size());

			// Loop over all triangles
			for (int i = 0; i < indices.size() / 3; i++) {
				// Get the indices of all the vertices of the triangles
				int v0 = indices[i * 3];
				int v1 = indices[i * 3 + 1];
				int v2 = indices[i * 3 + 2];

				// Calculate normal for all points.
				Vector3 n1 = Vector3::Cross(positions[v0] - positions[v2], positions[v1] - positions[v0]);
				Vector3 n2 = Vector3::Cross(positions[v1] - positions[v0], positions[v2] - positions[v1]);
				Vector3 n3 = Vector3::Cross(positions[v2] - positions[v1], positions[v0] - positions[v2]);

				Vector3 n = (n1 + n2 + n3) / 3;

				normals.emplace_back(n);
			}
		}

		void UpdateTransforms()
		{
			//Calculate Final Transform 
			const Transformation finalTransform = scaleTransform.append(rotationTransform.append(translationTransform));
			minAABB = Vector3{ FLT_MAX, FLT_MAX, FLT_MAX };
			maxAABB = Vector3{ FLT_MIN, FLT_MIN , FLT_MIN };

			//Transform Positions (positions > transformedPositions)
			if (transformedPositions.size() > 1) {
				transformedPositions.clear();
			}
			else {
				transformedPositions.reserve(positions.size());
			}
			for (int i = 0; i < positions.size(); i++) {
				transformedPositions.emplace_back(finalTransform.transformPoint(positions[i]));
				minAABB = Vector3::Min(minAABB, transformedPositions[i]);
				maxAABB = Vector3::Max(maxAABB, transformedPositions[i]);
			}


			//Transform Normals (normals > transformedNormals)
			if (transformedNormals.size() > 1) {
				transformedNormals.clear();
			}
			else {
				transformedNormals.reserve(positions.size());
			}
			for (int i = 0; i < normals.size(); i++) {
				transformedNormals.emplace_back(finalTransform.transformVector(normals[i]));
			}
		}
	};
}
