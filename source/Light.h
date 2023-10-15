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
		ColorRGB GetRadiance(const Vector3& target) const;
	};
	#pragma endregion


	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			return light.origin - origin;
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3
			assert(false && "No Implemented Yet!");
			return {};
		}
	}
}
