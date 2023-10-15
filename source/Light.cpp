#include "Light.h"

namespace dae {
    Ray Light::CreateLightRay(Vector3 origin) const
    {
		Vector3 direction = LightUtils::GetDirectionToLight(*this, origin);
		Ray lightRay = Ray(origin, direction.Normalized());

		float max = type == LightType::Point ? direction.Magnitude() : FLT_MAX;
		lightRay.max = max;
		lightRay.min = 0.01f;

        return lightRay;
    }

	Vector3 Light::GetDirectionToLight(const Vector3 target) const
	{
		if (type == LightType::Directional)
			return -direction;
		return origin - target;
	}

	ColorRGB Light::GetRadiance(const Vector3& target) const
	{
		return ColorRGB();
	}
}
