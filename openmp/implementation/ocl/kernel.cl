void atomicAdd_g_f(volatile __global float *addr, float val) {
    union {
        unsigned int u32;
        float f32;
    } next, expected, current;
    current.f32 = *addr;
    do {
        expected.f32 = current.f32;
        next.f32 = expected.f32 + val;
        current.u32 = atomic_cmpxchg((volatile __global unsigned int *)addr,
                                     expected.u32, next.u32);
    } while (current.u32 != expected.u32);
}

float calculateDistance(float x1, float y1,
                        float x2, float y2) {
    const float xDiff = x1 - x2;
    const float yDiff = y1 - y2;
    return xDiff * xDiff + yDiff * yDiff;
}

__kernel void findClosestClusters(
    // Input args
    __read_only __global float *inPoints,
    __read_only __global float *inCentroids,
    uint inNumberOfCentroids,
    // Output args
    __write_only __global uint *outPointLabels,
    __write_only __global float *outCentroidSums,
    __write_only __global uint *outCentroidDivisors) {

    // Get my point
    const uint pointIndex = get_global_id(0);
    const float pointX = inPoints[pointIndex * 2 + 0];
    const float pointY = inPoints[pointIndex * 2 + 1];

    // Iterate over clusters to find the closest one
    float minDistance = 1e300;
    uint selectedLabel = 0;
    for (int clusterIndex = 0; clusterIndex < inNumberOfCentroids; clusterIndex++) {
        const float centroidX = inCentroids[clusterIndex * 2 + 0];
        const float centroidY = inCentroids[clusterIndex * 2 + 1];
        const float distance = calculateDistance(pointX, pointY, centroidX, centroidY);

        if (distance < minDistance) {
            minDistance = distance;
            selectedLabel = clusterIndex;
        }
    }

    outPointLabels[pointIndex] = selectedLabel;
    atomicAdd_g_f(outCentroidSums + (2 * selectedLabel + 0), pointX);
    atomicAdd_g_f(outCentroidSums + (2 * selectedLabel + 1), pointY);
    atomicAdd_g_f(outCentroidDivisors + selectedLabel, 1);
}

__kernel void updateCentroids(
    // Input args
    __read_only __global float *inCentroidSums,
    __read_only __global uint *inCentroidDivisors,
    __read_only __global float *inCentroids,
    uint inNumberOfPoints,
    // Output args
    __write_only __global uint *outCentroids,
    __write_only __global uint *outCentroidsChanged) {

    // Get my centroid
    const uint centroidIndex = get_global_id(0);
    const uint centroidDivisor = inCentroidDivisors[centroidIndex];
    if (centroidDivisor == 0) {
        return;
    }

    // Calculate new position
    const float inCentroidSumX = inCentroidSums[centroidIndex * 2 + 0];
    const float inCentroidSumY = inCentroidSums[centroidIndex * 2 + 1];
    const float newCentroidX = inCentroidSumX / centroidDivisor;
    const float newCentroidY = inCentroidSumY / centroidDivisor;
    outCentroids[centroidIndex * 2 + 0] = newCentroidX;
    outCentroids[centroidIndex * 2 + 1] = newCentroidY;

    // Get previous position and early-return if it's the same
    const float oldCentroidX = inCentroids[centroidIndex * 2 + 0];
    const float oldCentroidY = inCentroids[centroidIndex * 2 + 1];
    if (newCentroidX == oldCentroidX && newCentroidY == oldCentroidY) {
        return;
    }

    // Centroids changed, signal it
    atomic_or(outCentroidsChanged, 1);
}
