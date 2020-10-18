mkdir build -p
cd build
cmake ..

if [ "$?" != 1 ]; then
    read _
fi
