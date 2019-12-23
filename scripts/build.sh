#!/bin/bash
BASE_CMD="cmake . -Bbuild"
BASE_BUILD="cd build && make"
BASE_INSTALL="make install"

if [ "$1" = "--test" ]
then
  export CMAKE_TARGET="TEST"
  $BASE_CMD -DCMAKE_BUILD_TYPE=Debug && $BASE_BUILD
elif [ "$1" = "--debug" ]
then
  export CMAKE_TARGET="BUILD"
  $BASE_CMD -DCMAKE_BUILD_TYPE=Debug && $BASE_BUILD
else
  export CMAKE_TARGET="BUILD"
  $BASE_CMD -DCMAKE_BUILD_TYPE=Release && $BASE_BUILD && $BASE_INSTALL
fi
