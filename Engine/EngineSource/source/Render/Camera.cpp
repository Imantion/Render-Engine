#include "Render/Camera.h"
#include "Math/math.h"
# define PI 3.14159265358979323846f

Engine::Camera::Camera(float verticalFov, float nearPlane, float farPlane)
{
	FOV = verticalFov;
	nearClip = nearPlane;
	farClip = farPlane;

	calculateViewMatrix();
}

Engine::vec3 Engine::Camera::getRayDirection(const vec2& point)
{
	vec3 upInterpolation = rayDirections[LeftUp] * (1 - point.x) + rayDirections[RightUp] * point.x;
	vec3 downInterpolation = rayDirections[LeftDown] * (1 - point.x) + rayDirections[RightDown] * point.x;

	return (downInterpolation * (1 - point.y) + upInterpolation * point.y).normalized();
}

Engine::vec3 Engine::Camera::calculateRayDirection(const vec2& screenPosition) const
{
	vec4 target = vec4(screenPosition.x, screenPosition.y, 1.0f, 1.0f) * inverseProjection;
	vec3 rayDirection = vec4((vec3(target) / target.w).normalized(), 0.0f) * inverseView;

	return rayDirection;
}

void Engine::Camera::calculateProjectionMatrix(int viewportWidth, int viewportHeight)
{
	projection = projectionMatrix(FOV * PI / 180.0f, nearClip, farClip, viewportWidth, viewportHeight);
	inverseProjection = mat4::Inverse(projection);
}

void Engine::Camera::calculateViewMatrix()
{
	view = viewMatrix(position,forwardDirection, upDirection);
	inverseView = mat4::Inverse(view);
}

void Engine::Camera::calculateRayDirections()
{
	vec2 coord(-1.0f);
	vec4 target = vec4(coord.x, coord.y, 1.0f, 1.0f) * inverseProjection;
	vec3 rayDirection = vec4((vec3(target) / target.w).normalized(), 0.0f) * inverseView;
	rayDirections[LeftDown] = rayDirection;

	coord.x = -1.0f;
	coord.y = 1.0f;;
	target = vec4(coord.x, coord.y, 1.0f, 1.0f) * inverseProjection;
	rayDirection = vec4((vec3(target) / target.w).normalized(), 0.0f) * inverseView;
	rayDirections[LeftUp] = rayDirection;

	coord.x = 1.0f;
	coord.y = 1.0f;;
	target = vec4(coord.x, coord.y, 1.0f, 1.0f) * inverseProjection;
	rayDirection = vec4((vec3(target) / target.w).normalized(), 0.0f) * inverseView;
	rayDirections[RightUp] = rayDirection;

	coord.x = 1.0f;
	coord.y = -1.0f;
	target = vec4(coord.x, coord.y, 1.0f, 1.0f) * inverseProjection;
	rayDirection = vec4((vec3(target) / target.w).normalized(), 0.0f) * inverseView;
	rayDirections[RightDown] = rayDirection;
}
//
//std::vector<Engine::vec3> Engine::Camera::getCameraFrustrum() const
//{
//	vec3 nearCenter = position - forwardDirection * nearClip;
//	vec3 farCenter = position - forwardDirection * farClip;
//
//	float nearHeight = 2 * tan(FOV * PI / (2 * 180.f)) * nearClip;
//	float farHeight = 2 * tan(FOV * PI / (2 * 180.f)) * farClip;
//	vec3 nearWidth = nearHeight * aspectRatio;
//	vec3 farWidth = farHeight * aspectRatio;
//
//	vec3 farTopLeft = farCenter + upDirection * (farHeight * 0.5) - rightDirection * (farWidth * 0.5);
//	vec3 farTopRight = farCenter + upDirection * (farHeight * 0.5) + rightDirection * (farWidth * 0.5);
//	vec3 farBottomLeft = farCenter - upDirection * (farHeight * 0.5) - rightDirection * (farWidth * 0.5);
//	vec3 farBottomRight = farCenter - upDirection * (farHeight * 0.5) + rightDirection * (farWidth * 0.5);
//
//	vec3 nearTopLeft = nearCenter + upDirection * (nearHeight * 0.5) - rightDirection * (nearWidth * 0.5);
//	vec3 nearTopRight = nearCenter + upDirection * (nearHeight * 0.5) + rightDirection * (nearWidth * 0.5);
//	vec3 nearBottomLeft = nearCenter - upDirection * (nearHeight * 0.5) - rightDirection * (nearWidth * 0.5);
//	vec3 nearBottomRight = nearCenter - upDirection * (nearHeight * 0.5) + rightDirection * (nearWidth * 0.5);
//
//	std::vector<vec3> a;
//	a.push_back((farTopLeft - nearTopLeft).normalized());
//	a.push_back((farTopRight - nearTopRight).normalized());
//	a.push_back((farBottomRight - nearBottomRight).normalized());
//	a.push_back((farBottomLeft - nearBottomLeft).normalized());
//	
//
//	return a;
//}

void Engine::Camera::setForward(vec3 f)
{
	forwardDirection = f;
}

void Engine::Camera::setUp(vec3 u)
{
	upDirection = u;
}

void Engine::Camera::setRight(vec3 r)
{
	rightDirection = r;
}
