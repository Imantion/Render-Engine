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

	return downInterpolation * (1 - point.y) + upInterpolation * point.y;
}

Engine::vec3 Engine::Camera::calculateRayDirection(const vec2& screenPosition)
{
	vec4 target = vec4(screenPosition.x, screenPosition.y, 1, 1) * inverseProjection;
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
	inverseView = transformMatrix(position, forwardDirection, rightDirection, upDirection);
	view = viewMatrix(position, position + forwardDirection, upDirection);
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
