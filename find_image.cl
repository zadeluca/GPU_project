///
//  find_image.cl
//
//  This kernel can be used to determine if a target image exists within a chip
//  (or subset) of a larger image.
//
//  In the current implementation, the "target image" is a white pixel.

__kernel void find_image(
	__read_only image2d_t image,
	uint imageWidth,
	uint imageHeight,
	uint chipWidth,
	uint chipHeight,
    __global uint* output)
{
	int x_id = get_global_id(0);
	int y_id = get_global_id(1);

	// output is a 2-D array with dimensions matching the global work size.
	// This means that each work item can output 1 if the target image
	// is found within its image chip, or 0 otherwise.
	int out_id = get_global_size(0) * y_id + x_id;

	uchar found = 0;

	// x and y interate the 2-D location of each pixel within the chip
    for (int y = chipHeight * y_id; y < chipHeight * (y_id + 1) && y < imageHeight; y++) {

        for (int x = chipWidth * x_id; x < chipWidth * (x_id + 1) && x < imageWidth; x++) {

			uint4 pixel = read_imageui(image, (int2)(x, y));

			// Check if pixel is white
			if (pixel.s0 == 255 && pixel.s1 == 255 && pixel.s2 == 255) {
				//printf("Found:  (%d, %d)\n", x, y);
				found = 1;
			}
        }
    }

    //printf("x_id = %d, y_id = %d, out_id = %d, found = %u\n", x_id, y_id, out_id, found);

    output[out_id] = found;
}