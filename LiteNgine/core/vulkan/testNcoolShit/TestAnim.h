#pragma once
#include <glm/glm.hpp>
#include<glm/vec2.hpp>
#include<glm/vec3.hpp>
#include <vulkan/vulkan_raii.hpp>
#include<array>
#include<tuple>
#include "../VertexHandler.h"
#include<iostream>
namespace lte {

	class TestAnim
	{
	public:
		TestAnim();
		~TestAnim();
		void interpolate(int frame);
		std::vector<Vertex> vertices = {
		{{0.0f, -0.5f}, {1.0f, 0.0f, 1.0f}},
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.8f}},
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}} };
	private:

		int fps = 60;
		int counter;
		bool repeat = true;
		std::vector<std::tuple<float, float, float, int>> colorV3 = {
			{ 0.1f,0.1f,0.1f,0},
			{ 0.4f,0.2f,0.4f,32},
			{ 0.8f,0.9f,0.3f,80},
			{ 0.5f,1.0f,0.7f,120}
		};
		std::vector<std::tuple<float, float, float, int>> colorV2 = {
			{ 0.1f,0.1f,0.1f,0},
			{ 0.4f,0.2f,0.4f,22},
			{ 0.8f,0.9f,0.6f,60},
			{ 0.5f,1.0f,0.7f,120}
		};
		std::vector<std::tuple<float, float, float, int>> colorV1 = {
			{ 0.1f,0.1f,0.1f,0},
			{ 0.4f,0.2f,0.4f,22},
			{ 0.8f,0.9f,0.6f,60},
			{ 0.3f,0.6f,0.3f,80},
			{ 0.5f,1.0f,0.7f,120}
		};
		int maxFrame = 120;
		
		void bakeAnim();
		
	};

}
