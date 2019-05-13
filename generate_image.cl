///
//  generate_image.cl
//
//  This kernel can be used to insert a target image at a specific location
//  within a larger image.
//
//  In the current implementation, the "target image" is a white pixel.

__kernel void generate_image(
	uint x,
	uint y,
	__write_only image2d_t image)
{
	//printf("Target: (%u, %u)\n", x, y);

	// Write a white pixel to target coordinates
	write_imageui(image, (int2)(x, y), (uint4)(255, 255, 255, 0));	
}