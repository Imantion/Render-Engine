#pragma once
#include <stdint.h>
#include <vector>
#include <memory>
#include "Math/math.h"
#include "Math/Mesh.h"
#include "Render/Light/Light.h"

namespace Engine
{
	class BVH;
	class Window;
	class Camera;

	class Scene
	{
	public:

		struct cube : primitive {
			static std::unique_ptr<mesh> m_mesh;

			virtual const Engine::mesh* getMesh() override
			{
				return m_mesh.get();
			}
		};

		Scene();

		void render(Window& window, Camera& camera);

		void redraw(bool redraw) { redrawScene = redraw; }
		vec2 getBR() const { return BR; }
		vec2 getTL() const { return TL; }

		bool intersectSpheres(const ray& r, hitInfo& hInfo, objectRef& isectObject);
		bool intersectPrimitive(const ray& r, hitInfo& hInfo, objectRef& isectObject);
		Material CheckIntersection(const ray& r, hitInfo& hInfo, objectRef& object);

		size_t getSphereAmount()
		{
			return spheres.size();
		}

		sphere& getSphere(int index)
		{
			return spheres[index];
		}

		size_t getPrimitivesAmount()
		{
			return cubes.size();
		}

		primitive& getPrimitive(int index)
		{
			return cubes[index];
		}

	private:

		uint32_t PerPixel(int x, int y);


		bool redrawScene;
		vec2 TL, BR;

		
		std::vector<sphere> spheres;
		std::vector<cube> cubes;
		plane infinitePlane;

		PointLight pointLight;
		SpotLight spotLight;
		DirectionalLight sunlight;
		
		Camera* s_camera;

		std::vector<uint32_t> verticalIterator;
		std::vector<uint32_t> horizontalIterator;
	};
}
