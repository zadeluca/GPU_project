# GPU_project
This program can be used to determine if a target image exists within a larger image.

Processing is done in parallel on the GPU using OpenCL.

In the current implementation, the image is a black background and the "target image" is a white pixel.


Sample output of run.sh:

```
Compiling with 'make'...
g++ find_image.cpp -o find_image.exe -I /opt/amdgpu-pro/include/ -I /usr/local/cuda-8.0/targets/x86_64-linux/include/ -L /opt/amdgpu-pro/lib/x86_64-linux-gnu/ -lOpenCL --std=c++11

Usage: ./find_image.exe IMAGE_WIDTH IMAGE_HEIGHT CHIP_WIDTH CHIP_HEIGHT NUM_ITERATIONS [debug]

Executing test #1...

Image size = 1920x1080, chip size = 1920x1080, numIterations = 100 and debug = 0

Total Execution Time:     53023 ms
Corrected Execution Time: 53010 ms
Time Per Find:            530.1 ms

Score (Finds/second):     1.88644


Executing test #2...

Image size = 1920x1080, chip size = 1000x1000, numIterations = 100 and debug = 0

Total Execution Time:     25492 ms
Corrected Execution Time: 25478 ms
Time Per Find:            254.78 ms

Score (Finds/second):     3.92495


Executing test #3...

Image size = 1920x1080, chip size = 800x800, numIterations = 100 and debug = 0

Total Execution Time:     16338 ms
Corrected Execution Time: 16325 ms
Time Per Find:            163.25 ms

Score (Finds/second):     6.12557


Executing test #4...

Image size = 1920x1080, chip size = 700x700, numIterations = 100 and debug = 0

Total Execution Time:     12498 ms
Corrected Execution Time: 12484 ms
Time Per Find:            124.84 ms

Score (Finds/second):     8.01025


Executing test #5...

Image size = 1920x1080, chip size = 600x600, numIterations = 100 and debug = 0

Total Execution Time:     9177 ms
Corrected Execution Time: 9163 ms
Time Per Find:            91.63 ms

Score (Finds/second):     10.9135


Executing test #6...

Image size = 1920x1080, chip size = 500x500, numIterations = 100 and debug = 0

Total Execution Time:     6040 ms
Corrected Execution Time: 6027 ms
Time Per Find:            60.27 ms

Score (Finds/second):     16.592


Executing test #7...

Image size = 1920x1080, chip size = 400x400, numIterations = 100 and debug = 0

Total Execution Time:     3867 ms
Corrected Execution Time: 3854 ms
Time Per Find:            38.54 ms

Score (Finds/second):     25.9471


Executing test #8...

Image size = 1920x1080, chip size = 300x300, numIterations = 100 and debug = 0

Total Execution Time:     2194 ms
Corrected Execution Time: 2181 ms
Time Per Find:            21.81 ms

Score (Finds/second):     45.8505


Executing test #9...

Image size = 1920x1080, chip size = 200x200, numIterations = 100 and debug = 0

Total Execution Time:     1017 ms
Corrected Execution Time: 1003 ms
Time Per Find:            10.03 ms

Score (Finds/second):     99.7009


Executing test #10...

Image size = 1920x1080, chip size = 120x120, numIterations = 100 and debug = 0

Total Execution Time:     413 ms
Corrected Execution Time: 399 ms
Time Per Find:            3.99 ms

Score (Finds/second):     250.627


Executing test #11...

Image size = 1920x1080, chip size = 60x60, numIterations = 100 and debug = 0

Total Execution Time:     168 ms
Corrected Execution Time: 154 ms
Time Per Find:            1.54 ms

Score (Finds/second):     649.351


Executing test #12...

Image size = 1920x1080, chip size = 40x40, numIterations = 100 and debug = 0

Total Execution Time:     151 ms
Corrected Execution Time: 138 ms
Time Per Find:            1.38 ms

Score (Finds/second):     724.638


Executing test #13...

Image size = 1920x1080, chip size = 30x30, numIterations = 100 and debug = 0

Total Execution Time:     139 ms
Corrected Execution Time: 126 ms
Time Per Find:            1.26 ms

Score (Finds/second):     793.651


Executing test #14...

Image size = 1920x1080, chip size = 20x20, numIterations = 100 and debug = 0

Total Execution Time:     131 ms
Corrected Execution Time: 117 ms
Time Per Find:            1.17 ms

Score (Finds/second):     854.701


Executing test #15...

Image size = 1920x1080, chip size = 15x15, numIterations = 100 and debug = 0

Total Execution Time:     125 ms
Corrected Execution Time: 111 ms
Time Per Find:            1.11 ms

Score (Finds/second):     900.901


Executing test #16...

Image size = 1920x1080, chip size = 12x12, numIterations = 100 and debug = 0

Total Execution Time:     125 ms
Corrected Execution Time: 112 ms
Time Per Find:            1.12 ms

Score (Finds/second):     892.857


Executing test #17...

Image size = 1920x1080, chip size = 10x10, numIterations = 100 and debug = 0

Total Execution Time:     126 ms
Corrected Execution Time: 112 ms
Time Per Find:            1.12 ms

Score (Finds/second):     892.857


Executing test #18...

Image size = 1920x1080, chip size = 8x8, numIterations = 100 and debug = 0

Total Execution Time:     127 ms
Corrected Execution Time: 113 ms
Time Per Find:            1.13 ms

Score (Finds/second):     884.956


Executing test #19...

Image size = 1920x1080, chip size = 6x6, numIterations = 100 and debug = 0

Total Execution Time:     130 ms
Corrected Execution Time: 116 ms
Time Per Find:            1.16 ms

Score (Finds/second):     862.069


Executing test #20...

Image size = 1920x1080, chip size = 5x5, numIterations = 100 and debug = 0

Total Execution Time:     133 ms
Corrected Execution Time: 119 ms
Time Per Find:            1.19 ms

Score (Finds/second):     840.336


Executing test #21...

Image size = 1920x1080, chip size = 4x4, numIterations = 100 and debug = 0

Total Execution Time:     136 ms
Corrected Execution Time: 122 ms
Time Per Find:            1.22 ms

Score (Finds/second):     819.672


Executing test #22...

Image size = 1920x1080, chip size = 3x3, numIterations = 100 and debug = 0

Total Execution Time:     148 ms
Corrected Execution Time: 134 ms
Time Per Find:            1.34 ms

Score (Finds/second):     746.269


Executing test #23...

Image size = 1920x1080, chip size = 2x2, numIterations = 100 and debug = 0

Total Execution Time:     199 ms
Corrected Execution Time: 186 ms
Time Per Find:            1.86 ms

Score (Finds/second):     537.634


Executing test #24...

Image size = 1920x1080, chip size = 1x1, numIterations = 100 and debug = 0

Total Execution Time:     706 ms
Corrected Execution Time: 692 ms
Time Per Find:            6.92 ms

Score (Finds/second):     144.509
```