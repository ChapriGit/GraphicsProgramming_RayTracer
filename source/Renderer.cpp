//External includes
#include "SDL.h"
#include "SDL_surface.h"
#include <iostream>

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
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	// For each pixel
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			// CREATE RAY
			Ray hitRay = CalculateRay(px, py, camera, camera.origin);

			// Set up Color to write to buffer
			ColorRGB finalColor{};

			// HitRecord containing information about a potential hit
			HitRecord closestHit{};
			pScene->GetClosestHit(hitRay, closestHit);

			if (closestHit.didHit) {
				//if (px == 353 && py == 400) {
				//	std::cout << "Oi";
				//}
				finalColor = m_colorManager.CalculateColor(pScene, &closestHit, hitRay.direction);
			}

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

Ray Renderer::CalculateRay(int x, int y, const Camera& camera, const Vector3& origin) const
{
	// Calculate Ray direction
	float pcx = x + 0.5f;
	float pcy = y + 0.5f;

	// cx = (2 (px+0.5) / width - 1) (width/height)
	// cy = 1 - 2 (py + 0.5) / height
	float cx = 2 * pcx / m_Width - 1;
	cx *= (float)m_Width / (float)m_Height;
	float cy = 1 - (2 * pcy) / m_Height;

	float angleRad = camera.fovAngle * PI / 180.f;
	float fov = tan(angleRad / 2.f);
	cx *= fov;
	cy *= fov;

	// r = cx right + cy up + look
	// Normalise the result

	Matrix cameraTransform = camera.cameraToWorld;
	Vector3 rayDirection = cameraTransform.TransformVector({cx, cy, 1}).Normalized();

	return Ray(origin, rayDirection);
}

ColorRGB ColorManager::CalculateColor(Scene* pScene, HitRecord* hit, Vector3 viewDir) const
{
	ColorRGB color{};

	switch (this->m_currentLightingMode) {
	//case(LightingMode::Radiance):
	//	color = pScene->GetRadiance(hit, m_ShadowsEnabled);
	//	break;

	//case (LightingMode::ObservedArea):
	//	color = pScene->GetObservedArea(hit, m_ShadowsEnabled);
	//	break;

	//case (LightingMode::Combined):
	//	color = pScene->GetColour(hit, m_ShadowsEnabled, viewDir);
	//	break;

	default:
		color = pScene->GetBRDF(hit, m_ShadowsEnabled, viewDir);
		break;
	}
	color.MaxToOne();
	
	return color;
}
