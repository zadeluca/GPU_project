///
//  find_image.cl
//
//  This kernel can be used to determine if a target image exists
//  within a chip (or subset) of a larger image.
//
//  TODO: Currently, this implementation is a simple proof of concept
//        in which the "image" is a 2-D array of 0s and 1s and the
//        "target image" is a 1.

__kernel void find_image(
	__global uint * image,
	uint imageWidth,
	uint imageHeight,
	uint chipWidth,
	uint chipHeight,
    __global uint * output)
{
	int x_id = get_global_id(0);
	int y_id = get_global_id(1);

	// output is a 2-D array with dimensions matching the global work size.
	// This means that each work item can output 1 if the target image
	// is found within its image chip, or 0 otherwise.
	int out_id = get_global_size(0) * y_id + x_id;

	bool found = false;

	// x and y interate the 2-D location of each pixel within the chip
    for (int y = chipHeight * y_id; y < chipHeight * (y_id + 1); y++) {

        for (int x = chipWidth * x_id; x < chipWidth * (x_id + 1); x++) {

			// image is a 2-D array stored in row-major order
			if (image[imageWidth * y + x]) {

				found = true;
			}
        }
    }

    output[out_id] = found;
}