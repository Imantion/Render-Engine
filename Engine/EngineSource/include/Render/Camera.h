#pragma once
#include "Math/matrix.h"
#include "Math/vec.h"

namespace Engine
{
	class Camera
	{
	public:

		Camera(){}
		Camera(float verticalFov, float nearPlane, float farPlane);

		vec3 getRayDirection(vec2 point);
		vec3 getPosition() { return position; }

		void calculateProjectionMatrix(int viewportWidth, int viewportHeight);
		void calculateViewMatrix();
		void calculateRayDirections();
		void moveCamera(const vec3& direction) { position += direction; }

		vec3 getForward() { return forwardDirection; }
		vec3 getUp() { return upDirection; }
		vec3 getRight() { return rightDirection; }

		void setForward(vec3 f);
		void setUp(vec3 u);
		void setRight(vec3 r);

	private:
		mat4 projection, inverseProjection;
		mat4 view, inverseView;
		float nearClip = 0.1f;
		float farClip = 100.0f;
		float FOV = 45.0f;
		
		vec3 position{ 0.0f, 0.0f, -2.0f };
		vec3 forwardDirection{ 0.0f,0.0f,1.0f };
		vec3 upDirection{ 0.0f,1.0f,0.0f };
		vec3 rightDirection{ 1.0f,0.0f,0.0f };

		enum { // Corners of frustum to orient in rayDirections array
			LeftUp,
			RightUp,
			LeftDown,
			RightDown
		};
		vec3 rayDirections[4]; // 4 corners of frustrum
	};
}