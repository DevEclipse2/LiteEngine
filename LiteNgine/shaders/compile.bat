C:\VulkanSDK\1.4.341.1\Bin\slangc.exe shader.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o slang.spv
pause
C:\VulkanSDK\1.4.341.1\Bin\slangc.exe skinnedShader.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertSkinned -o skin.spv
pause