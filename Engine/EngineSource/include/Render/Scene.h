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
	class Mesh;
	struct Material;

	class Scene
	{
	public:

		struct cube : primitive {
			static std::unique_ptr<Mesh> mesh;

			virtual const Mesh* getMesh() override
			{
				return mesh.get();
			}
		};

		Scene();

		void render(Window& window, Camera& camera);

		void moveSphere(vec3 direction);
		void setSpherePosition(vec3 position);

		void redraw(bool redraw) { redrawScene = redraw; }
		vec2 getBR() const { return BR; }
		vec2 getTL() const { return TL; }

		sphere* intersectSpheres(const ray& r, hitInfo& hInfo);
		primitive* intersectPrimitive(const ray& r, hitInfo& hInfo);

		int getSphereAmount()
		{
			return spheres.size();
		}

		sphere& getSphere(int index)
		{
			return spheres[index];
		}

		int getPrimitivesAmount()
		{
			return cubes.size();
		}

		primitive& getPrimitive(int index)
		{
			return cubes[index];
		}

	private:

		uint32_t PerPixel(int x, int y);
		Material CheckIntersection(const ray& r, hitInfo& hInfo);


		bool redrawScene;
		vec2 TL, BR;

		
		std::vector<sphere> spheres;
		std::vector<cube> cubes;

		PointLight pointLight;
		SpotLight spotLight;
		
		
		Camera* s_camera;

		std::vector<uint32_t> verticalIterator;
		std::vector<uint32_t> horizontalIterator;
	};
}
