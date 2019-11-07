# cleanup;
make clean
rm -rf CMakeCache.txt
rm -rf cmake_install.cmake

# build;
if [ "$1" = "--test" ]
then
    export CMAKE_TARGET="TEST"
    cmake . && make
else
    export CMAKE_TARGET="BUILD"
    cmake . && make && make install
fi
