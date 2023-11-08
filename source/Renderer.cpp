#define PARALLEL_EXECUTION

//External includes
#include "SDL.h"
#include "SDL_surface.h"
#include <iostream>
#include <execution>

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	const Matrix cameraToWorld = camera.CalculateCameraToWorld();

	const float aspectRatio = m_Width / static_cast<float>(m_Height);
	const float fovAngle = camera.fovAngle * TO_RADIANS;
	const float fov = tan(fovAngle / 2.f);

#if defined (PARALLEL_EXECUTION)
	//Parallel stuff
	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height)};
	std::vector<uint32_t> pixelIndices{};

	// Reserve a colour buffer
	pixelIndices.reserve(amountOfPixels);
	for (uint32_t index{}; index < amountOfPixels; ++index)
		pixelIndices.emplace_back(index);

	// Each pixel can de rendered in parallel
	std::for_each(std::execution::par, pixelIndices.begin(), pixelIndices.end(), [&](int i) {
		RenderPixel(pScene, i, fov, aspectRatio, cameraToWorld, camera.origin);
		});

#else
	// If no threads
	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) };
	for (uint32_t pixelIndex{}; pixelIndex < amountOfPixels; pixelIndex++) {
		RenderPixel(pScene, pixelIndex, fov, aspectRatio, cameraToWorld, camera.origin);
	}

#endif
	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void dae::Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Matrix cameraToWorld, const Vector3 cameraOrigin) const
{
	auto materials{ pScene->GetMaterials()};
	const uint32_t px{ pixelIndex % m_Width }, py{ pixelIndex / m_Width };

	// Find the pixel in camera space
	float rx{ px + 0.5f }, ry{ py + 0.5f };
	float cx{ (2 * (rx / float(m_Width)) - 1) * aspectRatio * fov };
	float cy{ (1 - (2 * (ry / float(m_Height)))) * fov };

	// Create a ray for the pixel
	Vector3 rayDirection = cameraToWorld.TransformVector({ cx, cy, 1 }).Normalized();
	Ray hitRay = Ray(cameraOrigin, rayDirection);

	// Set up Color to write to buffer
	ColorRGB finalColor{};

	// HitRecord containing information about a potential hit
	HitRecord closestHit{};
	pScene->GetClosestHit(hitRay, closestHit);

	if (closestHit.didHit) {
		finalColor = m_colorManager.CalculateColor(pScene, &closestHit, hitRay.direction);
	}

	//Update Color in Buffer
	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

ColorRGB ColorManager::CalculateColor(Scene* pScene, HitRecord* hit, const Vector3& viewDir) const
{
	ColorRGB color{};

	switch (this->m_currentLightingMode) {
	case(LightingMode::Radiance):
		color = pScene->GetRadiance(hit, m_ShadowsEnabled);
		break;

	case (LightingMode::ObservedArea):
		color = pScene->GetObservedArea(hit, m_ShadowsEnabled);
		break;

	case (LightingMode::Combined):
		color = pScene->GetColour(hit, m_ShadowsEnabled, viewDir);
		break;

	case (LightingMode::BRDF):
		color = pScene->GetBRDF(hit, m_ShadowsEnabled, viewDir);
		break;
	}
	color.MaxToOne();
	
	return color;
}
