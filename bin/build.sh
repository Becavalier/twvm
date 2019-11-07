# build;
if [ "$1" = "--test" ]
then
    export CMAKE_TARGET="TEST"
    cmake . && make
else
    export CMAKE_TARGET="BUILD"
    cmake . && make && make install
fi
