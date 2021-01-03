mkdir build -p
cd build
cmake ..

if [ "$?" != 0 ]; then
    read _
fi
