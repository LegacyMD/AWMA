# Measure time depending on numerOfPoints
for numberOfPoints in 10 1000 10000 100000 200000 300000 400000 500000 800000; do
    ./data_generation randomSeed=0 maxX=10000 maxY=10000 numberOfPoints=$numberOfPoints numberOfClusters=10 >/dev/null
    
    printf "%d, " $numberOfPoints
    for i in {1..10}; do
        result=`./kmeans_openmp maxIterations=10000 randomSeed=0 implementation=2 writeCsv=0 verbose=0 inputFileName=generated.txt`
        if [ $? != 0 ]; then
            printf "ERROR, "
        else
            printf "%d, " $result
        fi
    done
    echo
done

# Measure time depending on OMP_NUM_THREADS
./data_generation randomSeed=0 maxX=10000 maxY=10000 numberOfPoints=10000 numberOfClusters=10 >/dev/null
for numThreads in {1..16}; do
    export OMP_NUM_THREADS=$numThreads
    
    printf "%d, " $numThreads
    for i in {1..3}; do
        result=`./kmeans_openmp maxIterations=10000 randomSeed=0 implementation=1 writeCsv=0 verbose=0 inputFileName=generated.txt`
        if [ $? != 0 ]; then
            printf "ERROR, "
        else
            printf "%d, " $result
        fi
    done
    echo
done

# Measure iterations count
for numberOfPoints in 10 1000 10000 100000 200000 300000 400000 500000 800000 1000000; do
    ./data_generation randomSeed=0 maxX=10000 maxY=10000 numberOfPoints=$numberOfPoints numberOfClusters=10 >/dev/null
    
    printf "%d, " $numberOfPoints
    for i in {1..5}; do
        result=`./kmeans_openmp maxIterations=10000 implementation=1 writeCsv=0 verbose=1 inputFileName=generated.txt | grep -E "Iteration [0-9]+:" | cut -d' ' -f2 | tail -1 | sed s/://g`
        if [ $? != 0 ]; then
            printf "ERROR, "
        else
            printf "%d, " $result
        fi
    done
    echo
done


# Manual runs
./data_generation randomSeed=0 maxX=10000 maxY=10000 numberOfPoints=200000 numberOfClusters=10 >/dev/null
( touch ../openmp/implementation/implementation.cpp && make -j`nproc` && cd bin && for i in {1..5}; do ./kmeans_openmp maxIterations=10000 randomSeed=0 implementation=2 writeCsv=0 verbose=0 inputFileName=generated.txt; done )




# TRASH
./data_generation randomSeed=0 maxX=10000 maxY=10000 numberOfPoints=10000 numberOfClusters=10 >/dev/null
for numThreads in {1..16}; do
    export OMP_NUM_THREADS=$numThreads
    
    printf "%d, " $numThreads
    for i in {1..3}; do
        result=`./kmeans_openmp maxIterations=10000 randomSeed=0 implementation=1 writeCsv=0 verbose=0 inputFileName=generated.txt`
        if [ $? != 0 ]; then
            printf "ERROR, "
        else
            printf "%d, " $result
        fi
    done
    echo
done
./data_generation randomSeed=0 maxX=10000 maxY=10000 numberOfPoints=100000 numberOfClusters=10 >/dev/null
for numThreads in {1..16}; do
    export OMP_NUM_THREADS=$numThreads
    
    printf "%d, " $numThreads
    for i in {1..3}; do
        result=`./kmeans_openmp maxIterations=10000 randomSeed=0 implementation=2 writeCsv=0 verbose=0 inputFileName=generated.txt`
        if [ $? != 0 ]; then
            printf "ERROR, "
        else
            printf "%d, " $result
        fi
    done
    echo
done
