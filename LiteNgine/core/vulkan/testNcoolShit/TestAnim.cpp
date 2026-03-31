#include "TestAnim.h"
namespace lte {


	void TestAnim::bakeAnim() {

	}
	TestAnim::TestAnim() {

	}
	TestAnim::~TestAnim() {

	}
	void TestAnim::interpolate(int frame) {
		int frameCpy = frame;
		if (repeat) {
			frameCpy %= maxFrame;
		}
		std::vector<std::tuple<float, float, float, int>> colors;
		for (int i = 0; i < 3; i++) {
			switch (i) {
			case 0:
				colors = colorV1;
				break;
			case 1:
				colors = colorV2;
				break;
			case 2:
				colors = colorV3;
				break;
			}
			for (int j = 0; j < colors.size(); j++) {
				int framenumber = std::get<3>(colors[j]);
				if (framenumber < frameCpy) {
					continue;
				}
				else if (framenumber == frameCpy || j == 0) {
					
					glm::vec3 selectedColor = { std::get<0>(colors[j]) , std::get<1>(colors[j]) ,  std::get<2>(colors[j]) };
					vertices[i].color = selectedColor;
					break;
				}
				else {
					// larger
					//interpolation color a (prev keyframe) and color b (next keyframe) 
					//
					//std::cout << "interpolating \n";
					float interp = frameCpy / (std::get<3>(colors[j]) - std::get<3>(colors[j - 1]));
					float deltaR = (std::get<0>(colors[j]) - std::get<0>(colors[j - 1]));
					float deltaG = (std::get<1>(colors[j]) - std::get<1>(colors[j - 1]));
					float deltaB = (std::get<2>(colors[j]) - std::get<2>(colors[j - 1]));
					glm::vec3 selectedColor = { std::clamp( std::get<0>(colors[j-1]) + deltaR * interp , 0.0f, 1.0f),  std::clamp(std::get<1>(colors[j-1]) + deltaG * interp, 0.0f, 1.0f) ,  std::clamp(std::get<2>(colors[j-1]) + deltaB * interp , 0.0f, 1.0f)};
					vertices[i].color = selectedColor;

					break;
				}

			}
		}
		
	}

}