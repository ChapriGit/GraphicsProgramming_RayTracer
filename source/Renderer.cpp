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
			
			// Calculate Ray direction
			float pcx = px + 0.5f;
			float pcy = py + 0.5f;

			// cx = (2 (px+0.5) / width - 1) (width/height)
			// cy = 1 - 2 (py + 0.5) / height
			float cx = 2 * pcx / m_Width - 1;
			cx *= (float) m_Width / (float) m_Height;
			float cy = 1 - (2 * pcy) / m_Height;

			// r = cx right + cy up + look
			// Normalise the result
			Vector3 rayDirection = camera.forward + cx * camera.right + cy * camera.up;
			rayDirection.Normalize();

			Ray hitRay{ {0,0,0}, rayDirection };

			// Set up Color to write to buffer
			ColorRGB finalColor{};

			// HitRecord containing information about a potential hit
			HitRecord closestHit{};

			// TEMPORARY SPHERE
			Sphere testSphere{ {0.f, 0.f, 100.f}, 50.f, 0 };
			GeometryUtils::HitTest_Sphere(testSphere, hitRay, closestHit);

			if (closestHit.didHit) {
				const float scaled_t = (closestHit.t - 50.f) / 40.f;
				finalColor = scaled_t * materials[closestHit.materialIndex]->Shade();
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

Vector3 Renderer::CalculateRayDirection(int x, int y)
{
	return Vector3();
}
