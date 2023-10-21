#pragma once
#include <cassert>
#include "Math.h"
#include "vector"
#include "DataTypes.h"

namespace dae {

	#pragma region LIGHT
	enum class LightType
	{
		Point,
		Directional
	};

	struct Light
	{
		Vector3 origin{};
		Vector3 direction{};
		ColorRGB color{};
		float intensity{};

		LightType type{};

		Ray CreateLightRay(Vector3 origin) const;
		Vector3 GetDirectionToLight(const Vector3 origin) const;
	};
	#pragma endregion


	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			if (light.type == LightType::Directional)
				return light.direction.Normalized();
			return light.origin - origin;
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			if (light.type == LightType::Directional)
				return light.intensity * light.color;

			float distance = (light.origin - target).Magnitude();
			return light.color * light.intensity / (float) (distance * distance);
		}
	}
}
