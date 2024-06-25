#pragma once
#include "Math/matrix.h"
#include "Math/vec.h"
#include "vector"

namespace Engine
{
	class Camera
	{
	public:
		enum frustrumCorners { // Corners of frustum to orient in rayDirections array
			LeftUp,
			RightUp,
			LeftDown,
			RightDown
		};
	public:

		Camera();
		Camera(float verticalFov, float nearPlane, float farPlane);

		vec3 getRayDirection(const vec2&);
		vec3 calculateRayDirection(const vec2&) const;
		vec3 getPosition() { return position; }

		void calculateProjectionMatrix(int viewportWidth, int viewportHeight);
		void calculateViewMatrix();
		void calculateRayDirections();
		void moveCamera(const vec3& direction) { position += direction; }

		uint32_t getCameraTransformId() { return ID; };
		vec3 getForward() { return forwardDirection; }
		vec3 getUp() { return upDirection; }
		vec3 getRight() { return rightDirection; }
		const mat4& getViewMatrix() const;
		const mat4& getInverseViewMatrix() const;
		const mat4& getProjectionMatrix() const { return projection; }
		vec3 getCameraFrustrum(frustrumCorners fc);

		void setForward(vec3 f);
		void setUp(vec3 u);
		void setRight(vec3 r);

		

	private:
		mat4 projection, inverseProjection;
		uint32_t ID = -1;
		mat4 view;
		vec3 cameraFrustrum[3];
		float nearClip = 0.1f;
		float farClip = 100.0f;
		float FOV = 45.0f;
		float aspectRatio = 0.0f;
		
		vec3 position{ 0.0f, 0.0f, -2.0f };
		vec3 forwardDirection{ 0.0f,0.0f,1.0f };
		vec3 upDirection{ 0.0f,1.0f,0.0f };
		vec3 rightDirection{ 1.0f,0.0f,0.0f };

		
		vec3 rayDirections[4]; // 4 corners of frustrum
	};
}