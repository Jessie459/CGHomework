#pragma once
#ifndef SHIP_H
#define SHIP_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Ship_Movement {
	SFORWARD,
	SBACKWARD,
	SLEFT,
	SRIGHT,
	PITCHUP,
	PITCHDOWN,
	YAWLEFT,
	YAWRIGHT,
	ROLLLEFT,
	ROLLRIGHT
};

// Default camera values
const float YAWVALUE = 0.0f;
const float PITCHVALUE = 0.0f;
const float ROLLVALUE = 0.0f;
const float SSPEED = 2.5f;
const float SSENSITIVITY = 0.1f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Ship
{
public:
	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// Euler Angles
	float Yaw;
	float Pitch;
	float Roll;
	// Camera options
	float MovementSpeed;
	float MouseSensitivity;

	// Constructor with vectors
	Ship(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAWVALUE, float pitch = PITCHVALUE, float roll = ROLLVALUE) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SSPEED), MouseSensitivity(SSENSITIVITY)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		Roll = roll;
		updateShipVectors();
	}

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Ship_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;
		if (direction == SFORWARD)
			Position -= Right * velocity;
		if (direction == SBACKWARD)
			Position += Right * velocity;
		if (direction == SLEFT)
			Position -= Front * velocity;
		if (direction == SRIGHT)
			Position += Front * velocity;

		float angles = 0.01f;
		if (direction == YAWLEFT) {
			Yaw += angles;
		}
		if (direction == YAWRIGHT) {
			Yaw -= angles;
		}
		if (direction == PITCHUP) {
			Pitch += angles;
		}
		if (direction == PITCHDOWN) {
			Pitch -= angles;
		}
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		
		if (direction == ROLLLEFT) {
			Roll += angles;
		}
		if (direction == ROLLRIGHT) {
			Roll -= angles;
		}
		updateShipVectors();
	}



private:
	// Calculates the front vector from the Camera's (updated) Euler Angles
	void updateShipVectors()
	{
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
		
	}
};

#endif