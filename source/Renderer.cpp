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
			cx *= (m_Width / m_Height);
			float cy = 1 - (2 * pcy) / m_Height;

			// r = cx right + cy up + look
			// Normalise the result
			Vector3 rayDirection = camera.forward + cx * camera.right + cy * camera.up;
			rayDirection.Normalize();

			Ray hitRay{ {0,0,0}, rayDirection };

			ColorRGB finalColor{ rayDirection.x, rayDirection.y, rayDirection.z };

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