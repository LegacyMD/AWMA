#ifndef POINTS_STRIDE
#error NEED POINTS_STRIDE
#endif

#ifndef POINT_LABELS_STRIDE
#error NEED POINT_LABELS_STRIDE
#endif

#ifndef CENTROIDS_STRIDE
#error NEED CENTROIDS_STRIDE
#endif

__kernel void main(
    // Input args
    __read_only __global double *inPoints,
    __read_only __global uint *inPointLabels,
    __read_only __global double *inCentroids,
    uint inNumberOfPoints,
    uint inNumberOfCentroids,
    // Output args
    __write_only __global uint *outPointLabels,
    __write_only __global double *outCentroids) {
}
