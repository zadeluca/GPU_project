///
//  find_image.cpp
//
//  This program can be used to determine if a target image exists
//  within a larger image.
//
//  Processing is done in parallel on the GPU using OpenCL.
//
//  TODO: Currently, this implementation is a simple proof of concept
//        in which the "image" is a 2-D array of 0s and 1s and the
//        "target image" is a 1.

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

using namespace std;

// Constants
const char * kernelFile = "find_image.cl";
const char * kernelName = "find_image";

///
//  Print elapsed execution time of an OpenCL event
//  Adapted from OpenCL Programming Guide, pg. 330
//
void PrintElapsedEventTime(cl_event event, const char *msg) {
	cl_ulong startTime, endTime;

	clWaitForEvents(1, &event);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong),
			&startTime, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong),
			&endTime, NULL);
	clReleaseEvent(event);

	double runtime = (double) (endTime - startTime) * 1.0e-6;

	cout << msg << ": " << runtime << " ms\n";
}

///
// Function to check and handle OpenCL errors
inline void checkErr(cl_int err, const char * name) {
	if (err != CL_SUCCESS) {
		cerr << "ERROR: " << name << " (" << err << ")" << endl;
		exit (EXIT_FAILURE);
	}
}

///
//  Setup OpenCL and execute the kernel with specified parameters
//
int ExecuteKernel(unsigned int imageWidth, unsigned int imageHeight,
		unsigned int chipWidth, unsigned int chipHeight, bool debug) {
	cl_int errNum;
	cl_uint numPlatforms;
	cl_uint numDevices;
	cl_platform_id * platformIDs;
	cl_device_id * deviceIDs;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel kernel;
	cl_mem imageBuffer;
	cl_mem outputBuffer;
	cl_event event;

	unsigned int outputWidth = imageWidth / chipWidth;
	unsigned int outputHeight = imageHeight / chipHeight;

	cl_uint image[imageHeight][imageWidth];
	cl_uint output[outputHeight][outputWidth];

	cout << "\nExecuting Kernel with " << imageWidth << "x" << imageHeight
			<< " image, " << chipWidth << "x" << chipHeight
			<< " chip size, and debug=" << debug << endl;

	// First, select an OpenCL platform to run on.
	checkErr(clGetPlatformIDs(0, NULL, &numPlatforms), "clGetPlatformIDs");
	checkErr(numPlatforms > 0 ? CL_SUCCESS : -1, "No platforms available");
	platformIDs = (cl_platform_id *) alloca(
			sizeof(cl_platform_id) * numPlatforms);
	checkErr(clGetPlatformIDs(numPlatforms, platformIDs, NULL),
			"clGetPlatformIDs");

	// Iterate through the list of platforms until we find one that supports
	// a GPU device, otherwise fail with an error.
	deviceIDs = NULL;
	cl_uint i;
	for (i = 0; i < numPlatforms; i++) {
		errNum = clGetDeviceIDs(platformIDs[i], CL_DEVICE_TYPE_GPU, 0, NULL,
				&numDevices);
		if (errNum != CL_SUCCESS && errNum != CL_DEVICE_NOT_FOUND) {
			checkErr(errNum, "clGetDeviceIDs");
		} else if (numDevices > 0) {
			deviceIDs = (cl_device_id *) alloca(
					sizeof(cl_device_id) * numDevices);
			checkErr(
					clGetDeviceIDs(platformIDs[i], CL_DEVICE_TYPE_GPU,
							numDevices, deviceIDs, NULL), "clGetDeviceIDs");
			break;
		}
	}

	// Check to see if we found at least one GPU device
	checkErr(deviceIDs != NULL ? CL_SUCCESS : -1, "No GPU device found");

	// Next, create an OpenCL context on the selected platform.
	cl_context_properties contextProperties[] = { CL_CONTEXT_PLATFORM,
			(cl_context_properties) platformIDs[i], 0 };
	context = clCreateContext(contextProperties, numDevices, deviceIDs, NULL,
			NULL, &errNum);
	checkErr(errNum, "clCreateContext");

	// Pick the first device and create command queue.
	queue = clCreateCommandQueue(context, deviceIDs[0],
			CL_QUEUE_PROFILING_ENABLE, &errNum);
	checkErr(errNum, "clCreateCommandQueue");

	// Create program from kernel source
	ifstream srcFile(kernelFile);
	checkErr(srcFile.is_open() ? CL_SUCCESS : -1, kernelFile);
	string srcProg(istreambuf_iterator<char>(srcFile),
			(istreambuf_iterator<char>()));
	const char * src = srcProg.c_str();
	size_t length = srcProg.length();
	program = clCreateProgramWithSource(context, 1, &src, &length, &errNum);
	checkErr(errNum, "clCreateProgramWithSource");

	// Build program
	errNum = clBuildProgram(program, numDevices, deviceIDs, NULL, NULL, NULL);
	if (errNum != CL_SUCCESS) {
		// Determine the reason for the error
		char buildLog[16384];
		clGetProgramBuildInfo(program, deviceIDs[0], CL_PROGRAM_BUILD_LOG,
				sizeof(buildLog), buildLog, NULL);
		cerr << "Error in kernel: \n" << buildLog << endl;
		checkErr(errNum, "clBuildProgram");
	}

	// Create kernel object
	kernel = clCreateKernel(program, kernelName, &errNum);
	checkErr(errNum, "clCreateKernel");

	// Generate random image
	image[rand() % imageHeight][rand() % imageWidth] = 1;

	// Allocate image buffer
	imageBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY,
			sizeof(cl_uint) * imageHeight * imageWidth, NULL, &errNum);
	checkErr(errNum, "clCreateBuffer(image)");

	// Allocate output buffer
	outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
			sizeof(cl_uint) * outputHeight * outputWidth, NULL, &errNum);
	checkErr(errNum, "clCreateBuffer(output)");

	// Write the image buffer to the Device
	checkErr(
			clEnqueueWriteBuffer(queue, imageBuffer, CL_TRUE, 0,
					sizeof(cl_uint) * imageHeight * imageWidth, image, 0, NULL,
					&event), "clEnqueueWriteBuffer");
	PrintElapsedEventTime(event, "Copy Image Host->Device");

	// Set the kernel arguments
	errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &imageBuffer);
	errNum |= clSetKernelArg(kernel, 1, sizeof(cl_uint), &imageWidth);
	errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), &imageHeight);
	errNum |= clSetKernelArg(kernel, 3, sizeof(cl_uint), &chipWidth);
	errNum |= clSetKernelArg(kernel, 4, sizeof(cl_uint), &chipHeight);
	errNum |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &outputBuffer);
	checkErr(errNum, "clSetKernelArg");

	// Kernel uses 2-D work size, equal to the number of chips in the image
	const size_t globalWorkSize[2] = { outputWidth, outputHeight };
	const size_t localWorkSize[2] = { 1, 1 };

	// Queue the kernel up for execution across the array
	checkErr(
			clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalWorkSize,
					localWorkSize, 0, NULL, &event), "clEnqueueNDRangeKernel");
	PrintElapsedEventTime(event, "Kernel Execution");

	// Read the output back to the Host
	checkErr(
			clEnqueueReadBuffer(queue, outputBuffer, CL_TRUE, 0,
					sizeof(cl_uint) * outputHeight * outputWidth, output, 0,
					NULL, &event), "clEnqueueReadBuffer");
	PrintElapsedEventTime(event, "Copy Output Device->Host");

	if (debug) {
		cout << "\nImage:\n";
		for (int y = 0; y < imageHeight; y++) {
			for (int x = 0; x < imageWidth; x++) {
				cout << image[y][x] << " ";
			}
			cout << endl;
		}

		cout << "\nOutput:\n";
		for (int y = 0; y < outputHeight; y++) {
			for (int x = 0; x < outputWidth; x++) {
				cout << output[y][x] << " ";
			}
			cout << endl;
		}
	}

	// Release resources
	clReleaseMemObject(imageBuffer);
	clReleaseMemObject(outputBuffer);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
	if (argc < 5 || argc > 6) {
		cout << "Usage: " << argv[0]
				<< " IMAGE_WIDTH IMAGE_HEIGHT CHIP_WIDTH CHIP_HEIGHT [debug]\n";
		return EXIT_FAILURE;
	}

	// read command line arguments
	int imageWidth = atoi(argv[1]);
	int imageHeight = atoi(argv[2]);
	int chipWidth = atoi(argv[3]);
	int chipHeight = atoi(argv[4]);
	bool debug = (argc == 6 && (string(argv[5]) == "debug"));

	// validate command line arguments
	if (imageWidth <= 0) {
		cout << "Error: Image Width (" << argv[1]
				<< ") must be an integer > 0\n";
		return EXIT_FAILURE;
	}
	if (imageHeight <= 0) {
		cout << "Error: Image Height (" << argv[2]
				<< ") must be an integer > 0\n";
		return EXIT_FAILURE;
	}
	if (chipWidth <= 0) {
		cout << "Error: Chip Width (" << argv[3]
				<< ") must be an integer > 0\n";
		return EXIT_FAILURE;
	}
	if (chipHeight <= 0) {
		cout << "Error: Chip Height (" << argv[4]
				<< ") must be an integer > 0\n";
		return EXIT_FAILURE;
	}
	if (imageWidth % chipWidth != 0) {
		cout << "Error: Chip Width must evenly divide Image Width\n";
		return EXIT_FAILURE;
	}
	if (imageHeight % chipHeight != 0) {
		cout << "Error: Chip Height must evenly divide Image Height\n";
		return EXIT_FAILURE;
	}

	return ExecuteKernel(imageWidth, imageHeight, chipWidth, chipHeight, debug);
}
