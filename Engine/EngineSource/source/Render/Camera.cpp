#include "Render/Camera.h"
#include "Math/math.h"
#include "Graphics/TransformSystem.h"
# define PI 3.14159265358979323846f

Engine::Camera::Camera()
{
	ID = TransformSystem::Init()->AddModelTransform(TransformSystem::transforms{ mat4::Identity() }, 1u);
}

Engine::Camera::Camera(float verticalFov, float nearPlane, float farPlane)
{
	FOV = verticalFov;
	nearClip = nearPlane;
	farClip = farPlane;

	ID = TransformSystem::Init()->AddModelTransform(TransformSystem::transforms{ mat4::Identity()}, 1u);
	calculateViewMatrix();
}

Engine::vec3 Engine::Camera::getRayDirection(const vec2& point)
{
	vec3 upInterpolation = rayDirections[LeftUp] * (1 - point.x) + rayDirections[RightUp] * point.x;
	vec3 downInterpolation = rayDirections[LeftDown] * (1 - point.x) + rayDirections[RightDown] * point.x;

	return (downInterpolation * (1 - point.y) + upInterpolation * point.y).normalized();
}

Engine::vec3 Engine::Camera::calculateRayDirection(const vec2& screenPosition) const // not normalized
{
	vec4 target = vec4(screenPosition.x, screenPosition.y, 1.0f, 1.0f) * inverseProjection;
	vec3 rayDirection = vec4((vec3(target) / target.w), 0.0f) * TransformSystem::Init()->GetModelTransforms(ID)[0].modelToWold;

	return rayDirection;
}

void Engine::Camera::calculateProjectionMatrix(int viewportWidth, int viewportHeight)
{
	if (viewportWidth == 0 || viewportHeight == 0)
		return;

	projection = projectionMatrix(FOV * PI / 180.0f, nearClip, farClip, viewportWidth, viewportHeight);
	aspectRatio = (float)viewportWidth / (float)viewportHeight;
	inverseProjection = mat4::Inverse(projection);
}

void Engine::Camera::calculateViewMatrix()
{
	view = viewMatrix(position, forwardDirection, upDirection);
	TransformSystem::Init()->GetModelTransforms(ID)[0].modelToWold = mat4::Inverse(view);
}

void Engine::Camera::calculateRayDirections()
{
	mat4& inverseView = TransformSystem::Init()->GetModelTransforms(ID)[0].modelToWold;

	vec2 coord(-1.0f);
	vec4 target = vec4(coord.x, coord.y, 1.0f, 1.0f) * inverseProjection;
	vec3 rayDirection = vec4((vec3(target) / target.w), 0.0f) * inverseView;
	rayDirections[LeftDown] = rayDirection;

	coord.x = -1.0f;
	coord.y = 1.0f;;
	target = vec4(coord.x, coord.y, 1.0f, 1.0f) * inverseProjection;
	rayDirection = vec4((vec3(target) / target.w), 0.0f) * inverseView;
	rayDirections[LeftUp] = rayDirection;

	coord.x = 1.0f;
	coord.y = 1.0f;;
	target = vec4(coord.x, coord.y, 1.0f, 1.0f) * inverseProjection;
	rayDirection = vec4((vec3(target) / target.w), 0.0f) * inverseView;
	rayDirections[RightUp] = rayDirection;

	coord.x = 1.0f;
	coord.y = -1.0f;
	target = vec4(coord.x, coord.y, 1.0f, 1.0f) * inverseProjection;
	rayDirection = vec4((vec3(target) / target.w), 0.0f) * inverseView;
	rayDirections[RightDown] = rayDirection;
}

const Engine::mat4& Engine::Camera::getViewMatrix() const
{
	return view; 
}

const Engine::mat4& Engine::Camera::getInverseViewMatrix() const
{
	return TransformSystem::Init()->GetModelTransforms(ID)[0].modelToWold;;
}


Engine::vec3 Engine::Camera::getCameraFrustrum(frustrumCorners fc) // They are not normilized. Because in another case relation beetwen cornerns won't stay
{
	return rayDirections[fc];
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

Engine::vec3 Engine::Camera::getPosition()
{
	auto& transform = Engine::TransformSystem::Init()->GetModelTransforms(ID);

	return (vec3&)*transform[0].modelToWold[3];
}
