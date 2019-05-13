///
//  find_image.cpp
//
//  This program can be used to determine if a target image exists within a
//  larger image.
//
//  Processing is done in parallel on the GPU using OpenCL.
//
//  In the current implementation, the image is a black background and the
//  "target image" is a white pixel.

#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <cmath>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

using namespace std;

// Constants
const string GENERATE_KERNEL_FILE = "generate_image.cl";
const string GENERATE_KERNEL_NAME = "generate_image";
const string FIND_KERNEL_FILE = "find_image.cl";
const string FIND_KERNEL_NAME = "find_image";

///
// Function to check and handle OpenCL errors
//
inline void checkErr(cl_int err, string name) {
	if (err != CL_SUCCESS) {
		cerr << "ERROR: " << name << " (" << err << ")" << endl;
		exit (EXIT_FAILURE);
	}
}

///
//  Get elapsed execution time of an OpenCL event
//  Adapted from OpenCL Programming Guide, pg. 330
//
//  NOTE: Timing begins when the event is queued (intended for marker events)
//
double GetElapsedMarkerEventTime(cl_event event) {
	cl_ulong startTime, endTime;

	// Wait for the event to complete
	clWaitForEvents(1, &event);

	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_QUEUED,
			sizeof(cl_ulong), &startTime, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong),
			&endTime, NULL);

	return (double) (endTime - startTime) * 1.0e-6;
}

///
//  Create an OpenCL context on the first available GPU.
//
cl_context CreateContext(cl_device_id &deviceID) {
	cl_int errNum;
	cl_uint numPlatforms;
	cl_uint numDevices;
	cl_platform_id * platformIDs;
	cl_context context;

	// First, select an OpenCL platform to run on.
	checkErr(clGetPlatformIDs(0, NULL, &numPlatforms), "clGetPlatformIDs");
	checkErr(numPlatforms > 0 ? CL_SUCCESS : -1, "No platforms available");
	platformIDs = (cl_platform_id *) alloca(
			sizeof(cl_platform_id) * numPlatforms);
	checkErr(clGetPlatformIDs(numPlatforms, platformIDs, NULL),
			"clGetPlatformIDs");

	// Iterate through the list of platforms until we find one that supports
	// a GPU device, otherwise fail with an error.
	deviceID = NULL;
	cl_uint i;
	for (i = 0; i < numPlatforms; i++) {
		errNum = clGetDeviceIDs(platformIDs[i], CL_DEVICE_TYPE_GPU, 0, NULL,
				&numDevices);
		if (errNum != CL_SUCCESS && errNum != CL_DEVICE_NOT_FOUND) {
			checkErr(errNum, "clGetDeviceIDs");
		} else if (numDevices > 0) {
			checkErr(
					clGetDeviceIDs(platformIDs[i], CL_DEVICE_TYPE_GPU, 1,
							&deviceID, NULL), "clGetDeviceIDs");
			break;
		}
	}

	// Check to see if we found a GPU device
	checkErr(deviceID != NULL ? CL_SUCCESS : -1, "No GPU device found");

	// Next, create an OpenCL context on the selected platform.
	cl_context_properties contextProperties[] = { CL_CONTEXT_PLATFORM,
			(cl_context_properties) platformIDs[i], 0 };
	context = clCreateContext(contextProperties, 1, &deviceID, NULL, NULL,
			&errNum);
	checkErr(errNum, "clCreateContext");

	return context;
}

///
//  Create an OpenCL program from the kernel source file
//
cl_program CreateProgram(cl_context context, cl_device_id deviceID,
		string file) {
	cl_int errNum;
	cl_program program;

	ifstream srcFile(file);
	checkErr(srcFile.is_open() ? CL_SUCCESS : -1, file);

	string srcProg(istreambuf_iterator<char>(srcFile),
			(istreambuf_iterator<char>()));
	const char * src = srcProg.c_str();
	size_t length = srcProg.length();

	program = clCreateProgramWithSource(context, 1, &src, &length, &errNum);
	checkErr(errNum, "clCreateProgramWithSource");

	// Build program
	errNum = clBuildProgram(program, 1, &deviceID, NULL, NULL, NULL);
	if (errNum != CL_SUCCESS) {
		// Determine the reason for the error
		char buildLog[16384];
		clGetProgramBuildInfo(program, deviceID, CL_PROGRAM_BUILD_LOG,
				sizeof(buildLog), buildLog, NULL);
		cerr << "Error in kernel: \n" << buildLog << endl;
		checkErr(errNum, "clBuildProgram");
	}

	return program;
}

///
//  Create an OpenCL image
//
cl_mem CreateImage(cl_context context, unsigned int width,
		unsigned int height) {
	cl_image_format clImageFormat;
	clImageFormat.image_channel_order = CL_RGBA;
	clImageFormat.image_channel_data_type = CL_UNSIGNED_INT8;

	cl_image_desc clImageDesc;
	clImageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
	clImageDesc.image_width = width;
	clImageDesc.image_height = height;
	clImageDesc.image_depth = 0;
	clImageDesc.image_array_size = 0;
	clImageDesc.image_row_pitch = 0;
	clImageDesc.image_slice_pitch = 0;
	clImageDesc.num_mip_levels = 0;
	clImageDesc.num_samples = 0;
	clImageDesc.buffer = NULL;

	cl_int errNum;
	cl_mem clImage = clCreateImage(context, CL_MEM_READ_WRITE, &clImageFormat,
			&clImageDesc, NULL, &errNum);
	checkErr(errNum, "clCreateImage");

	return clImage;
}

///
//  Setup OpenCL and execute the kernels with specified parameters
//
int RunTest(unsigned int imageWidth, unsigned int imageHeight,
		unsigned int chipWidth, unsigned int chipHeight,
		unsigned int numIterations, bool debug, bool doFind) {

	cl_int errNum;
	cl_device_id deviceID;
	cl_context context;
	cl_mem imageBuffer, outputBuffer;
	cl_command_queue queue;
	cl_program generateProgram, findProgram;
	cl_kernel generateKernel, findKernel;

	// Compute output dimensions
	// The use ceil accounts for chip sizes that do not evenly divide the image
	unsigned int outputWidth = ceil((float) imageWidth / chipWidth);
	unsigned int outputHeight = ceil((float) imageHeight / chipHeight);
	unsigned int outputSize = outputWidth * outputHeight * sizeof(unsigned int);

	// Create OpenCL context - deviceID is passed by reference and set on return
	context = CreateContext(deviceID);

	// Create image
	imageBuffer = CreateImage(context, imageWidth, imageHeight);

	// Allocate output buffer
	outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, outputSize, NULL,
			&errNum);
	checkErr(errNum, "clCreateBuffer");

	// Create command queue for selected device
	cl_queue_properties properties[] = { CL_QUEUE_PROPERTIES,
			CL_QUEUE_PROFILING_ENABLE, 0 };
	queue = clCreateCommandQueueWithProperties(context, deviceID, properties,
			&errNum);
	checkErr(errNum, "clCreateCommandQueue");

	// Create programs from kernel source
	generateProgram = CreateProgram(context, deviceID, GENERATE_KERNEL_FILE);
	findProgram = CreateProgram(context, deviceID, FIND_KERNEL_FILE);

	// Needed to fill image with all 0s (black pixels)
	unsigned int fill_color[4] = { 0, 0, 0, 0 };
	size_t origin[3] = { 0, 0, 0 };
	size_t region[3] = { imageWidth, imageHeight, 1 };

	// Create kernel object for generating images
	generateKernel = clCreateKernel(generateProgram,
			GENERATE_KERNEL_NAME.c_str(), &errNum);
	checkErr(errNum, "clCreateKernel(generate)");

	// Set the kernel arguments for generating images
	// NOTE: Arguments 0 and 1 are set later
	checkErr(clSetKernelArg(generateKernel, 2, sizeof(cl_mem), &imageBuffer),
			"clSetKernelArg(generate-1)");

	// Images are generated 1 at a time
	const size_t global1[1] = { 1 };

	// Create kernel object for finding the target image
	findKernel = clCreateKernel(findProgram, FIND_KERNEL_NAME.c_str(), &errNum);
	checkErr(errNum, "clCreateKernel(find)");

	// Set the kernel arguments for finding the target image
	errNum = clSetKernelArg(findKernel, 0, sizeof(cl_mem), &imageBuffer);
	errNum |= clSetKernelArg(findKernel, 1, sizeof(cl_uint), &imageWidth);
	errNum |= clSetKernelArg(findKernel, 2, sizeof(cl_uint), &imageHeight);
	errNum |= clSetKernelArg(findKernel, 3, sizeof(cl_uint), &chipWidth);
	errNum |= clSetKernelArg(findKernel, 4, sizeof(cl_uint), &chipHeight);
	errNum |= clSetKernelArg(findKernel, 5, sizeof(cl_mem), &outputBuffer);
	checkErr(errNum, "clSetKernelArg(find)");

	// Finding uses 2-D work size, equal to the number of chips in the image
	//
	// NOTE: Testing with larger work group sizes has shown no improvements.
	// This may be a result of how the find kernel is currently implemented.
	const size_t globalWorkSize[2] = { outputWidth, outputHeight };
	const size_t localWorkSize[2] = { 1, 1 };

	// This user event will be used to delay execution until all operations have
	// been added to the queue. This is necessary to obtain accurate timing.
	cl_event begin = clCreateUserEvent(context, &errNum);
	checkErr(errNum, "clCreateUserEvent");

	// Enqueue barrier event - waits for 'begin' to complete
	checkErr(clEnqueueBarrierWithWaitList(queue, 1, &begin, NULL),
			"clEnqueueBarrierWithWaitList");

	// NOTE: It is not necessary to manually synchronize between all of the
	// following operations. There is only a single command queue which was not
	// configured to allow out of order execution, and therefore all commands
	// will fully execute, in order.
	for (int i = 0; i < numIterations; i++) {
		// Fill Image with all 0s (black pixels)
		checkErr(
				clEnqueueFillImage(queue, imageBuffer, fill_color, origin,
						region, 0, NULL, NULL), "clEnqueueFillImage");

		// Write random white pixel to image
		unsigned int x = rand() % imageWidth;
		unsigned int y = rand() % imageHeight;
		errNum = clSetKernelArg(generateKernel, 0, sizeof(cl_uint), &x);
		errNum |= clSetKernelArg(generateKernel, 1, sizeof(cl_uint), &y);
		checkErr(errNum, "clSetKernelArg(generate-2)");
		checkErr(
				clEnqueueNDRangeKernel(queue, generateKernel, 1, NULL, global1,
						NULL, 0, NULL, NULL),
				"clEnqueueNDRangeKernel(generate)");

		// Option to skip the find operation in order to time fill/generate only
		// This is needed to be able to exclude fill/generate from timing
		if (doFind) {
			// Find white pixel in image
			checkErr(
					clEnqueueNDRangeKernel(queue, findKernel, 2, NULL,
							globalWorkSize, localWorkSize, 0, NULL, NULL),
					"clEnqueueNDRangeKernel(find)");
		}
	}

	// Enqueue marker event - used for timing
	cl_event marker;
	checkErr(clEnqueueMarkerWithWaitList(queue, 0, 0, &marker),
			"clEnqueueMarkerWithWaitList");

	// Complete begin event to release barrier event
	clSetUserEventStatus(begin, CL_COMPLETE);

	// Total execution time
	double time = GetElapsedMarkerEventTime(marker);

	// NOTE: Debug info will only be displayed for the last iteration
	if (debug && doFind) {
		cout << "\nOutput size = " << outputWidth << "x" << outputHeight << endl;
		unsigned int output[outputHeight][outputWidth] = { };

		// Read the output back to the Host
		checkErr(
				clEnqueueReadBuffer(queue, outputBuffer, CL_TRUE, 0, outputSize,
						output, 0, NULL, NULL), "clEnqueueReadBuffer");

		int found = 0;
		for (int y = 0; y < outputHeight; y++) {
			for (int x = 0; x < outputWidth; x++) {
				if (output[y][x]) {
					found++;
				}
				cout << output[y][x] << " ";
			}
			cout << endl;
		}
		cout << "Found = " << found << endl;
	}

	// Release resources
	clReleaseEvent(marker);
	clReleaseEvent(begin);
	clReleaseKernel(findKernel);
	clReleaseKernel(generateKernel);
	clReleaseProgram(findProgram);
	clReleaseProgram(generateProgram);
	clReleaseCommandQueue(queue);
	clReleaseMemObject(outputBuffer);
	clReleaseMemObject(imageBuffer);
	clReleaseContext(context);

	return time;
}

int main(int argc, char **argv) {
	if (argc < 6 || argc > 7) {
		cout << "Usage: " << argv[0]
				<< " IMAGE_WIDTH IMAGE_HEIGHT CHIP_WIDTH CHIP_HEIGHT NUM_ITERATIONS [debug]\n";
		return EXIT_FAILURE;
	}

	// read command line arguments
	int imageWidth = atoi(argv[1]);
	int imageHeight = atoi(argv[2]);
	int chipWidth = atoi(argv[3]);
	int chipHeight = atoi(argv[4]);
	int numIterations = atoi(argv[5]);
	bool debug = (argc == 7 && (string(argv[6]) == "debug"));

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
	if (numIterations <= 0) {
		cout << "Error: Number of Iterations (" << argv[5]
				<< ") must be an integer > 0\n";
		return EXIT_FAILURE;
	}

	cout << "\nImage size = " << imageWidth << "x" << imageHeight
			<< ", chip size = " << chipWidth << "x" << chipHeight
			<< ", numIterations = " << numIterations << " and debug = " << debug
			<< "\n";

	double totalTime = RunTest(imageWidth, imageHeight, chipWidth, chipHeight,
			numIterations, debug, true);
	double noFindTime = RunTest(imageWidth, imageHeight, chipWidth, chipHeight,
			numIterations, debug, false);
	double correctedTime = totalTime - noFindTime;

	cout << endl;
	cout << "Total Execution Time:     " << totalTime << " ms\n";
	cout << "Corrected Execution Time: " << correctedTime << " ms\n";
	cout << "Time Per Find:            " << correctedTime / numIterations << " ms\n\n";
	cout << "Score (Finds/second):     " << numIterations / correctedTime * 1000 << "\n\n";

	return EXIT_SUCCESS;
}
