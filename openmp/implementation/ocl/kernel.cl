void atomic_add_float(volatile __global float *source, const float operand) {
    union {
        unsigned int intVal;
        float floatVal;
    } newVal;
    union {
        unsigned int intVal;
        float floatVal;
    } prevVal;
    do {
        prevVal.floatVal = *source;
        newVal.floatVal = prevVal.floatVal + operand;
    } while (atomic_cmpxchg((volatile __global unsigned int *)source, prevVal.intVal, newVal.intVal) != prevVal.intVal);
}

#define DATATYPE float

DATATYPE calculateDistance(DATATYPE x1, DATATYPE y1,
                           DATATYPE x2, DATATYPE y2) {
    const DATATYPE xDiff = x1 - x2;
    const DATATYPE yDiff = y1 - y2;
    return xDiff * xDiff + yDiff * yDiff;
}

__kernel void findClosestClusters(
    // Input args
    __read_only __global DATATYPE *inPoints,
    __read_only __global DATATYPE *inCentroids,
    uint inNumberOfCentroids,
    // Output args
    __write_only __global uint *outPointLabels,
    __write_only __global DATATYPE *outCentroidSums,
    __write_only __global uint *outCentroidDivisors) {

    // Get my point
    const uint pointIndex = get_global_id(0);
    const DATATYPE pointX = inPoints[pointIndex * 2 + 0];
    const DATATYPE pointY = inPoints[pointIndex * 2 + 1];

    // Iterate over clusters to find the closest one
    DATATYPE minDistance = 1e300;
    uint selectedLabel = 0;
    for (int clusterIndex = 0; clusterIndex < inNumberOfCentroids; clusterIndex++) {
        const DATATYPE centroidX = inCentroids[clusterIndex * 2 + 0];
        const DATATYPE centroidY = inCentroids[clusterIndex * 2 + 1];
        const DATATYPE distance = calculateDistance(pointX, pointY, centroidX, centroidY);

        if (distance < minDistance) {
            minDistance = distance;
            selectedLabel = clusterIndex;
        }
    }

    outPointLabels[pointIndex] = selectedLabel;
    atomic_add_float(outCentroidSums + (2 * selectedLabel + 0), pointX);
    atomic_add_float(outCentroidSums + (2 * selectedLabel + 1), pointY);
    atomic_inc(outCentroidDivisors + selectedLabel);
}

__kernel void updateCentroids(
    // Input args
    __read_only __global DATATYPE *inCentroidSums,
    __read_only __global uint *inCentroidDivisors,
    __read_only __global DATATYPE *inCentroids,
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
    const DATATYPE inCentroidSumX = inCentroidSums[centroidIndex * 2 + 0];
    const DATATYPE inCentroidSumY = inCentroidSums[centroidIndex * 2 + 1];
    const DATATYPE newCentroidX = inCentroidSumX / centroidDivisor;
    const DATATYPE newCentroidY = inCentroidSumY / centroidDivisor;
    outCentroids[centroidIndex * 2 + 0] = newCentroidX;
    outCentroids[centroidIndex * 2 + 1] = newCentroidY;

    // Get previous position and early-return if it's the same
    const DATATYPE oldCentroidX = inCentroids[centroidIndex * 2 + 0];
    const DATATYPE oldCentroidY = inCentroids[centroidIndex * 2 + 1];
    if (newCentroidX == oldCentroidX && newCentroidY == oldCentroidY) {
        return;
    }

    // Centroids changed, signal it
    atomic_or(outCentroidsChanged, 1);
}
