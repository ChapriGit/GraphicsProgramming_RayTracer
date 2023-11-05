#pragma once
#include "Matrix.h"
#include "DataTypes.h"
#include "math.h"

namespace dae {
	struct Transformation
	{
	private:
		Matrix matrix{};
		Matrix inverse{};

	public:
		Transformation() = default;
		Transformation(Matrix m, Matrix inv) : matrix(m), inverse{inv} {};

		Transformation append(const Transformation& t) const {
			return Transformation(matrix * t.matrix, t.inverse * inverse);
		}

		Vector3 transformPoint(const Vector3& p) const {
			return matrix.TransformPoint(p);
		}

		Vector3 transformVector(const Vector3& v) const {
			return matrix.TransformVector(v);
		}

		Ray inverseTransformRay(const Ray& r) const {
			Vector3 origin = inverse.TransformPoint(r.origin);
			Vector3 dir = inverse.TransformVector(r.direction);
			Ray ray = Ray();
			ray.origin = origin;
			ray.direction = dir;
			return ray;
		}

		static Transformation translate(float x, float y, float z) {
			Matrix transform = Matrix::CreateTranslation(x, y, z);
			Matrix inverse = Matrix::CreateTranslation(-x, -y, -z);
			return Transformation(transform, inverse);
		}

		static Transformation scale(float x, float y, float z) {
			Matrix transform = Matrix::CreateScale(x, y, z);
			Matrix inverse = Matrix::CreateScale(1/x, 1/y, 1/z);
			return Transformation(transform, inverse);
		}

		static Transformation rotateX(float angle) {
			Matrix transform = Matrix::CreateRotationX(angle);
			Matrix inverse = transform.Transpose();

			return Transformation(transform, inverse);
		}

		static Transformation rotateY(float angle) {
			Matrix transform = Matrix::CreateRotationY(angle);
			Matrix inverse = transform.Transpose();

			return Transformation(transform, inverse);
		}

		static Transformation rotateZ(float angle) {
			Matrix transform = Matrix::CreateRotationZ(angle);
			Matrix inverse = transform.Transpose();

			return Transformation(transform, inverse);
		}

	};
}

