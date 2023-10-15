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

	class LightManager {
	public:
		LightManager() = default;
		~LightManager() = default;

		LightManager(const LightManager&) = delete;
		LightManager(LightManager&&) noexcept = delete;
		LightManager& operator=(const LightManager&) = delete;
		LightManager& operator=(LightManager&&) noexcept = delete;

		void CycleLightingMode() {
			m_currentLightingMode = static_cast<LightingMode>((m_currentLightingMode + 1));
			if (m_currentLightingMode == 4)
				m_currentLightingMode = ObservedArea;
		};
		void ToggleShadows() { m_ShadowsEnabled = !m_ShadowsEnabled; }

		ColorRGB CalculateColor(Scene* pScene, HitRecord* pHit);

	private:
		enum LightingMode {
			ObservedArea,
			Radiance,
			BRDF,
			Combined
		};

		LightingMode m_currentLightingMode{ LightingMode::Combined };
		bool m_ShadowsEnabled{ true };
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

		LightManager lightManager{};

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};

		Ray CalculateRay(int x, int y, const Camera& camera, const Vector3& origin) const;
	};
}
