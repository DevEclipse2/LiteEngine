#include "ImageEditor.h"
#include <algorithm>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <iostream>

namespace lte
{
	void ImageEditor::ExportImage()
	{
		if (finalImage.imageData != nullptr ) delete[] finalImage.imageData;

		ParseFinalSize();
		finalImage.width = FinalSize.x;
		finalImage.height = FinalSize.y;
		finalImage.imageData = new uint8_t[FinalSize.x * FinalSize.y * 4];
		//assigns a block of memory here

		for (auto& layer : Layers)
		{
			//for each layer, bottommost to highest 
			//sorts the images in each layer from lowest to highest then writes into the pointer's location + offset.
			//redundant memory ops such as overwrites can be skipped
			std::vector<std::pair<uint16_t, TextureImage>> sorted_vec(layer.begin(), layer.end());

			std::sort(sorted_vec.begin(), sorted_vec.end(),
				[](const auto& a, const auto& b) {
					return a.second.sortOrder < b.second.sortOrder;
				}
			);
			for (auto& pair : sorted_vec)
			{
				uint8_t* pointer = finalImage.imageData;
				if (pair.second.position.y > 0)
				{
					pointer += (pair.second.position.y - 1) * finalImage.width * 4;
				}
				pointer += pair.second.position.x * 4;
				//moves pointer to correct position
				// begin memory writing operation
				for (int i = 0; i < pair.second.height; i++)
				{
					memcpy(pointer, pair.second.imageData, pair.second.width * 4);
					//writes a single row of pixels
					pointer += finalImage.width * 4;
				}
				
			}

		}
		int stride_in_bytes = finalImage.width * 4;

		// Export the image to PNG
		int success = stbi_write_png(
			"output_image.png",		// File path
			finalImage.width,		// Image width
			finalImage.height,      // Image height
			4,						// channels
			finalImage.imageData,   // image pointer
			stride_in_bytes			// byte strde
		);

		if (success) {
			std::cout << "Successfully saved output_image.png\n";
		}
		else {
			std::cerr << "Failed to save the image.\n";
		}
	}
	void ImageEditor::ParseFinalSize()
	{
		uint32_t minHeight;
		uint32_t maxHeight;
		uint32_t maxLength;
		uint32_t minLength;
		//this finds the dimensions of the thingy
		for (const auto& layer : Layers)
		{
			for (const auto& image : layer)
			{
				
				TextureImage textureImage = image.second;
				if (minHeight > textureImage.position.y - textureImage.height)
				{
					minHeight = textureImage.position.y - textureImage.height;
				}
				if (maxHeight < textureImage.position.y)
				{
					maxHeight = textureImage.position.y;
				}
				if (minLength > textureImage.position.x)
				{
					minLength = textureImage.position.x;
				}
			}
		}
		FinalSize = glm::vec2(maxLength - minLength, maxHeight - minHeight);

		

	}

}