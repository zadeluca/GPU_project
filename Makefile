all: find_image.cpp
	g++ find_image.cpp -o find_image.exe -I/opt/amdgpu-pro/include/ -I/usr/local/cuda-8.0/targets/x86_64-linux/include/ -L/opt/amdgpu-pro/lib/x86_64-linux-gnu/ -lOpenCL --std=c++11
