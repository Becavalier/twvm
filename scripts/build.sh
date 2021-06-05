#!/bin/bash
BUILD_VERSION=$(grep "version" ./package.json | awk -F \" '{print $4}')

if [ -n "$BUILD_VERSION" ] ; then
  export BUILD_VERSION
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
    export NDEBUG=1
    cmake . -Bbuild -DCMAKE_BUILD_TYPE=Release
    cd build
    make && make install
  fi
fi
