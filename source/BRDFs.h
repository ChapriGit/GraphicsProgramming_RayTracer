#pragma once
#include <cassert>
#include "Math.h"
#include <string>

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			return cd * kd / PI;
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			return cd * kd / PI;
		}

		/**
		 * \brief todo
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			Vector3 reflectance = Vector3::Reflect(-l, n);
			float cosine = Vector3::Dot(reflectance, -v);
			cosine = cosine < 0 ? 0 : cosine;
			return ks * pow(cosine, exp) * ColorRGB {1, 1, 1};
		}

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			float powerTerm = (float) pow(1 - Vector3::Dot(h, v), 5);
			return f0 + (ColorRGB{1, 1, 1} - f0) * powerTerm;
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			float alpha = Square(roughness);
			float denom = Square(Vector3::Dot(n, h)) * (alpha * alpha - 1) + 1;
			denom *= denom;
			denom *= PI;

			return (float) Square(alpha) / denom;
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
		{
			float alpha = Square(roughness);
			float k = (float) Square(alpha + 1) / 8.f;
			float nom = Vector3::Dot(n, v);
			float denom = nom * (1 - k);
			denom += k;

			if (denom < 0.01) {
				return 0;
			}

			float result = (float)nom / (float)denom;
			return result;
		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{
			float result = GeometryFunction_SchlickGGX(n, v, roughness) *
				GeometryFunction_SchlickGGX(n, l, roughness);

			result = result > 0 ? result : 0;
			return result;
		}

	}
}