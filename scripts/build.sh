#!/bin/bash
if [ "$1" = "--test" ]
then
  export CMAKE_TARGET="TEST"
  cmake . -Bbuild -DCMAKE_BUILD_TYPE=Debug
  cd build
  make
elif [ "$1" = "--debug" ]
then
  export CMAKE_TARGET="BUILD"
  cmake . -Bbuild -DCMAKE_BUILD_TYPE=Debug
  cd build
  make
else
  export CMAKE_TARGET="BUILD"
  cmake . -Bbuild -DCMAKE_BUILD_TYPE=Release
  cd build
  make && make install
fi
