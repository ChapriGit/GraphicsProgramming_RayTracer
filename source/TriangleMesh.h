#pragma once
#include "Math.h"
#include "vector"
#include "DataTypes.h"
#include "Transformation.h"
#include <iostream>

namespace dae
{
	struct BVHNode {
		Vector3 minAABB;
		Vector3 maxAABB;

		int leftChild;
		bool isLeaf;
		int firstTriangle, lastTriangle;
	};

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
		std::vector<BVHNode*> bvhNode{};
		std::vector<int> triangles{};


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
			for (int i = 0; i < indices.size() / 3; i++) {
				int v0 = indices[i * 3];
				int v1 = indices[i * 3 + 1];
				int v2 = indices[i * 3 + 2];
				Vector3 n1 = Vector3::Cross(positions[v0] - positions[v2], positions[v1] - positions[v0]);
				Vector3 n2 = Vector3::Cross(positions[v1] - positions[v0], positions[v2] - positions[v1]);
				Vector3 n3 = Vector3::Cross(positions[v2] - positions[v1], positions[v0] - positions[v2]);

				normals.emplace_back(n1);
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

		bool SlabTest_TriangleMesh(const Ray& ray) const {
			float tx1 = (minAABB.x - ray.origin.x) / ray.direction.x;
			float tx2 = (maxAABB.x - ray.origin.x) / ray.direction.x;

			float tmin = std::min(tx1, tx2);
			float tmax = std::max(tx1, tx2);

			float ty1 = (minAABB.y - ray.origin.y) / ray.direction.y;
			float ty2 = (maxAABB.y - ray.origin.y) / ray.direction.y;

			tmin = std::max(tmin, std::min(ty1, ty2));
			tmax = std::min(tmax, std::max(ty1, ty2));

			float tz1 = (minAABB.z - ray.origin.z) / ray.direction.z;
			float tz2 = (maxAABB.z - ray.origin.z) / ray.direction.z;

			tmin = std::max(tmin, std::min(tz1, tz2));
			tmax = std::min(tmax, std::max(tz1, tz2));

			return tmax > 0 && tmax >= tmin;
		}

		bool SlabTest_TriangleMesh(const Ray& ray, const Vector3& min, const Vector3& max) const {
			float tx1 = (min.x - ray.origin.x) / ray.direction.x;
			float tx2 = (max.x - ray.origin.x) / ray.direction.x;

			float tmin = std::min(tx1, tx2);
			float tmax = std::max(tx1, tx2);

			float ty1 = (min.y - ray.origin.y) / ray.direction.y;
			float ty2 = (max.y - ray.origin.y) / ray.direction.y;

			tmin = std::max(tmin, std::min(ty1, ty2));
			tmax = std::min(tmax, std::max(ty1, ty2));

			float tz1 = (min.z - ray.origin.z) / ray.direction.z;
			float tz2 = (max.z - ray.origin.z) / ray.direction.z;

			tmin = std::max(tmin, std::min(tz1, tz2));
			tmax = std::min(tmax, std::max(tz1, tz2));

			return tmax > 0 && tmax >= tmin;
		}

		void updateNodeBounds(int idx) {
			BVHNode* node = bvhNode[idx];
			Vector3 minValue = Vector3{ FLT_MAX, FLT_MAX, FLT_MAX };
			Vector3 maxValue = Vector3{ FLT_MIN, FLT_MIN, FLT_MIN };

			for (int i = node->firstTriangle; i <= node->lastTriangle; i++) {
				int triangle = triangles[i];

				int v0 = indices[triangle];
				int v1 = indices[triangle + 1];
				int v2 = indices[triangle + 2];

				minValue = Vector3::Min(minValue, positions[v0]);
				maxValue = Vector3::Max(maxValue, positions[v0]);
				minValue = Vector3::Min(minValue, positions[v1]);
				maxValue = Vector3::Max(maxValue, positions[v1]);
				minValue = Vector3::Min(minValue, positions[v2]);
				maxValue = Vector3::Max(maxValue, positions[v2]);
			}

			node->minAABB = minValue;
			node->maxAABB = maxValue;
		}

		void subdivideBVH(int idx) {
			BVHNode* node = bvhNode[idx];
			if (node->lastTriangle - node->firstTriangle <= 2) {
				node->isLeaf = true;
				return;
			}

			// Find axis to split. Currently going for SMS - Longest Axis
			Vector3 extent = node->maxAABB - node->minAABB;
			int axis = extent.y > extent.x ? 1 : 0;
			axis = extent.z > extent.y ? 2 : axis;

			float split = node->minAABB[axis] + extent[axis] * 0.5f;

			int i = node->firstTriangle;
			int j = node->lastTriangle;

			// Half sorting algorithm
			while (i <= j) {
				int triangle = triangles[i];

				int v0 = indices[triangle];
				int v1 = indices[triangle + 1];
				int v2 = indices[triangle + 2];

				Vector3 centre = (positions[v0] + positions[v1] + positions[v2]) * 0.333f;
				if (centre[axis] < split)
					i++;
				else {
					int tmp = triangles[i];
					triangles[i] = triangles[j];
					triangles[j] = tmp;
					j--;
				}
			}

			// If everything on one side, stop split
			int leftCount = i - node->firstTriangle;
			if (leftCount == 0 || j == node->lastTriangle) {
				node->isLeaf = true;
				return;
			}

			int nextNode = (int) bvhNode.size();
			bvhNode.push_back(new BVHNode{});
			bvhNode.push_back(new BVHNode{});

			BVHNode* leftChild = bvhNode[nextNode];
			BVHNode* rightChild = bvhNode[nextNode + 1];

			node->leftChild = nextNode;
			leftChild->firstTriangle = node->firstTriangle;
			leftChild->lastTriangle = j;
			updateNodeBounds(nextNode);
			subdivideBVH(nextNode);

			rightChild->firstTriangle = i;
			rightChild->lastTriangle = node->lastTriangle;
			updateNodeBounds(nextNode + 1);
			subdivideBVH(nextNode + 1);

		}

		void createBVH() {
			int nrTriangles = (int)indices.size() / 3;
			bvhNode.reserve(nrTriangles * 2 - 1);
			bvhNode.push_back(new BVHNode{});

			BVHNode& root = *bvhNode[0];
			root.leftChild = 0;
			root.firstTriangle = 0;
			root.lastTriangle = std::max(nrTriangles - 1, 1);

			triangles.reserve(nrTriangles);
			for (int i = 0; i < nrTriangles; i++)
				triangles.push_back(i * 3);

			updateNodeBounds(0);
			subdivideBVH(0);
		}

		std::vector<int> traverseTree(int idx, const Ray& ray) const {
			const BVHNode& node = *bvhNode[idx];
			std::vector<int> hit;
			hit.reserve(node.lastTriangle - node.firstTriangle);

			if (node.isLeaf) {
				for (int i = node.firstTriangle; i <= node.lastTriangle; i++) {
					hit.push_back(triangles[i]);
				}

				return hit;
			}

			const BVHNode* leftChild = bvhNode[node.leftChild];
			const BVHNode* rightChild = bvhNode[node.leftChild + 1];

			if (SlabTest_TriangleMesh(ray, leftChild->minAABB, leftChild->maxAABB)) {
				std::vector<int> leftHit = traverseTree(node.leftChild, ray);
				hit.insert(hit.end(), std::begin(leftHit), std::end(leftHit));
			}

			if (SlabTest_TriangleMesh(ray, rightChild->minAABB, rightChild->maxAABB)) {
				std::vector<int> rightHit = traverseTree(node.leftChild+1, ray);
				hit.insert(hit.end(), std::begin(rightHit), std::end(rightHit));
			}

			return hit;
		}
	};
}
