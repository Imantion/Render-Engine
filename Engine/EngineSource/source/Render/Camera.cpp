#include "Render/Camera.h"
#include "Math/math.h"
# define PI 3.14159265358979323846

Engine::Camera::Camera(float verticalFov, float nearPlane, float farPlane)
{
	FOV = verticalFov;
	nearClip = farPlane;
	farClip = farPlane;
}

Engine::vec3 Engine::Camera::getRayDirection(vec2 point)
{
	vec3 upInterpolation = rayDirections[LeftUp] * (1 - point.x) + rayDirections[RightUp] * point.x;
	vec3 downInterpolation = rayDirections[LeftDown] * (1 - point.x) + rayDirections[RightDown] * point.x;

	return downInterpolation * (1 - point.y) + upInterpolation * point.y;
}

void Engine::Camera::calculateProjectionMatrix(int viewportWidth, int viewportHeight)
{
	projection = projectionMatrix(FOV * PI / 180.0f, nearClip, farClip, viewportWidth, viewportHeight);
	inverseProjection = mat4::Inverse(projection);
}

void Engine::Camera::calculateViewMatrix()
{
	//view = viewMatrix(position, forwardDirection, rightDirection, upDirection);
	//inverseView = mat4::Inverse(view);
	/*inverseView = InverseLookAt(position, position + forwardDirection, upDirection);*/

	inverseView = viewMatrix(position, forwardDirection, rightDirection, upDirection);


}

void Engine::Camera::calculateRayDirections()
{
	vec2 coord(-1);
	vec4 target = vec4(coord.x, coord.y, 1, 1) * inverseProjection;
	vec3 rayDirection = vec4((vec3(target) / target.w).normalized(), 0) * inverseView;
	rayDirections[LeftDown] = rayDirection;

	coord.x = -1;
	coord.y = 1;;
	target = vec4(coord.x, coord.y, 1, 1) * inverseProjection;
	rayDirection = vec4((vec3(target) / target.w).normalized(), 0) * inverseView;
	rayDirections[LeftUp] = rayDirection;

	coord.x = 1;
	coord.y = 1;;
	target = vec4(coord.x, coord.y, 1, 1) * inverseProjection;
	rayDirection = vec4((vec3(target) / target.w).normalized(), 0) * inverseView;
	rayDirections[RightUp] = rayDirection;

	coord.x = 1;
	coord.y = -1;
	target = vec4(coord.x, coord.y, 1, 1) * inverseProjection;
	rayDirection = vec4((vec3(target) / target.w).normalized(), 0) * inverseView;
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
