#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <iostream>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		const float MOVEMENT_SPEED = 10.f;
		const float ROTATION_SPEED = 4.f;

		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{CalculateCameraToWorld()};


		Matrix CalculateCameraToWorld()
		{
			right = Vector3::Cross({ 0, 1, 0 }, forward.Normalized());
			right.Normalize();
			up = Vector3::Cross(forward, right);
			up.Normalize();

			return {right, up, forward.Normalized(), origin};
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			// Transform Rotation:
			bool rotated = false;
			Vector3 movementDirection = {};
			if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT) && mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) && abs(mouseY) > 1) {
				movementDirection += mouseY > 0 ? up : -up;
			}
			else {
				if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT) || mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) && abs(mouseX) > 1) {
					float rotationDist = mouseX > 0 ? SDL_clamp(mouseX / 800.f, -3, -1) : SDL_clamp(mouseX / 800.f, 1, 3);
					totalYaw -= rotationDist * ROTATION_SPEED;
					totalYaw = fmod(totalYaw, 360.f);
					rotated = true;
				}

				if ((mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) && abs(mouseY) > 1) {
					float rotationDist = mouseY > 0 ? SDL_clamp(mouseY / 800.f, -3, -1) : SDL_clamp(mouseY / 800.f, 1, 3);
					totalPitch += rotationDist * ROTATION_SPEED;
					totalPitch = fmod(totalPitch, 360.f);
					rotated = true;
				}
			}

			if (rotated) {
				Matrix finalRotation = Matrix::CreateRotation(totalPitch, totalYaw, 0);
				forward = finalRotation.TransformVector(Vector3::UnitZ);
				forward.Normalize();

				cameraToWorld = CalculateCameraToWorld();
			}


			// Transform Position
			if (pKeyboardState[SDL_SCANCODE_W] == 1)
				movementDirection = forward;
			if (pKeyboardState[SDL_SCANCODE_A] == 1)
				movementDirection -= right;
			if (pKeyboardState[SDL_SCANCODE_S] == 1)
				movementDirection -= forward;
			if (pKeyboardState[SDL_SCANCODE_D] == 1)
				movementDirection += right;
			if (pKeyboardState[SDL_SCANCODE_Q] == 1)
				movementDirection -= up;
			if (pKeyboardState[SDL_SCANCODE_E] == 1)
				movementDirection += up;

			if (movementDirection.Magnitude() > 0.001) {
				movementDirection.Normalize();

				origin += movementDirection * (deltaTime * MOVEMENT_SPEED);
			}

			else {
				if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT) && abs(mouseY) > 0.001) {
					float movementDist = mouseY > 0 ? SDL_clamp(mouseY / 800.f, -3, -1) : SDL_clamp(mouseY / 800.f, 1, 3);
					origin += forward * (deltaTime * MOVEMENT_SPEED * movementDist);
				}
			}


		}
	};
}
