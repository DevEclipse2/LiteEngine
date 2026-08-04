#pragma once
#include <cstdint>
#include <string>
#include "glm/glm.hpp"
namespace lte {	
	class ImageEditor
	{
	public:
		struct TextureImage
		{
			uint8_t* imageData = nullptr;
			uint16_t width = 0;
			uint16_t height = 0;
			std::string name = "Layer";
			glm::u32vec2 position = glm::u32vec2(0,0); //top left 
			uint16_t sortOrder = 0;
		};
		struct Group
		{
			std::vector<std::pair<uint16_t, uint16_t>> Children;
			glm::u32vec2 transform;
		};
		std::vector<Group> ImageGroups;
		std::vector<std::unordered_map<uint16_t,TextureImage>> Layers;// for this you can have a glmu16vec2
		std::vector<std::pair<glm::u32vec2,glm::u32vec2>> LayerFinalSize; //final size and offset
		
		//export helpers
		glm::u32vec2 FinalSize;
		void SubmitGuiCommands();
		void Update();
		void ExportLayers();
		void ExportImage();
		void ParseFinalSize();
		void Init();// creates pipeline and stuff

		/*inline void CheckPixel(uint8_t* dataPtr, uint8_t channels, glm::u8vec4 pixel, uint16_t width, glm::u16vec2 position)
		{
			//from the top down
			if (position.y != 0)
			{
				dataPtr += (position.y - 1) * width * channels;
			}
			dataPtr += position.x * channels;
			*dataPtr = pixel.r;
		}*/

	private:
		TextureImage finalImage{};
	};

}

//
//what image editor should be able to do
//import and drag textures around to align them
//lasso select to separate , duplicate, cut
//eraser and marker
//save settings
//the imageeditor chunks images into multiple smaller images and can use mipmaps 