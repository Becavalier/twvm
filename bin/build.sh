#!/bin/bash
if [ "$1" = "--test" ]
then
    export CMAKE_TARGET="TEST"
    cmake -DCMAKE_BUILD_TYPE=Release . && make
elif [ "$1" = "--debug" ]
then
    export CMAKE_TARGET="BUILD"
    cmake -DCMAKE_BUILD_TYPE=Debug . && make
else
    export CMAKE_TARGET="BUILD"
    cmake -DCMAKE_BUILD_TYPE=Release . && make && make install
fi
