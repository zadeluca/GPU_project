# GPU_project
This program can be used to determine if a target image exists within a larger image.

Processing is done in parallel on the GPU using OpenCL.

TODO: Currently, this implementation is a simple proof of concept in which the "image" is a 2-D array of 0s and 1s and the "target image" is a 1.


Sample output of run.sh:

```
Compiling with 'make'...
g++ find_image.cpp -o find_image.exe -I/opt/amdgpu-pro/include/ -I/usr/local/cuda-8.0/targets/x86_64-linux/include/ -L/opt/amdgpu-pro/lib/x86_64-linux-gnu/ -lOpenCL --std=c++11

Usage: ./find_image.exe IMAGE_WIDTH IMAGE_HEIGHT CHIP_WIDTH CHIP_HEIGHT [debug]

Executing small test...

Executing Kernel with 4x4 image, 2x2 chip size, and debug=1
Copy Image Host->Device: 0 ms
Kernel Execution: 0.01888 ms
Copy Output Device->Host: 0.01648 ms

Image:
0 0 0 0 
0 0 0 0 
0 1 0 0 
0 0 0 0 

Output:
0 0 
1 0 

Executing large test #1...

Executing Kernel with 1920x1080 image, 1920x1080 chip size, and debug=0
Copy Image Host->Device: 2.39664 ms
Kernel Execution: 477.561 ms
Copy Output Device->Host: 0.00176 ms

Executing large test #2...

Executing Kernel with 1920x1080 image, 24x24 chip size, and debug=0
Copy Image Host->Device: 2.35584 ms
Kernel Execution: 3.83776 ms
Copy Output Device->Host: 0.0096 ms
```