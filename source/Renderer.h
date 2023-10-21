#pragma once

#include <cstdint>
#include "Vector3.h"
#include "Camera.h"
#include <iostream>

#include "Utils.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;

	class ColorManager {
	public:
		ColorManager() = default;
		~ColorManager() = default;

		ColorManager(const ColorManager&) = delete;
		ColorManager(ColorManager&&) noexcept = delete;
		ColorManager& operator=(const ColorManager&) = delete;
		ColorManager& operator=(ColorManager&&) noexcept = delete;

		void CycleLightingMode() {
			m_currentLightingMode = static_cast<LightingMode>((m_currentLightingMode + 1));
			if (m_currentLightingMode == 4)
				m_currentLightingMode = ObservedArea;
			std::cout << "\n \n LIGHTING MODE : " << ToString(m_currentLightingMode) << std::endl;
		};

		void ToggleShadows() { m_ShadowsEnabled = !m_ShadowsEnabled; }

		ColorRGB CalculateColor(Scene* pScene, HitRecord* pHit, Vector3 viewDir) const;

	private:
		enum LightingMode {
			ObservedArea,
			Radiance,
			BRDF,
			Combined
		};

		LightingMode m_currentLightingMode{ LightingMode::Combined };
		bool m_ShadowsEnabled{ true };

		inline const char* ToString(LightingMode lm)
		{
			switch (lm)
			{
			case ObservedArea:  return "Observed Area";
			case Radiance:		return "Radiance";
			case BRDF:			return "BRDF";
			case Combined:		return "Combined";
			default:			return "Unknown";
			}
		}
	};

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;
		bool SaveBufferToImage() const;

		ColorManager m_colorManager{};

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};

		Ray CalculateRay(int x, int y, const Camera& camera, const Vector3& origin) const;
	};
}
