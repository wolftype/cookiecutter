#!/bin/bash

#Will compile to build/bin and run test/ccquote.cpp
#Will also make a copy of binary in local DIRECTORY

DIRECTORY=test
TARGET=ccquote
FILENAME=${TARGET}.cpp

MAKE_VERBOSE=0
RUN_TARGET=1
RUN_CMAKE=1

if [ $1 ]; then

  echo Compiling $1

  DIRECTORY=`dirname $1`
  TARGET=`basename $1|cut -d'.' -f1 | sed -e "s|/|_|g"`
  FILENAME=${TARGET}.cpp

  MAKE_VERBOSE=0
  RUN_TARGET=0
  RUN_CMAKE=0

fi

echo Directory is $DIRECTORY
echo Filename is $FILENAME
echo Target is $TARGET

for i
  do
    case $i in
    -q | --quiet)
      MAKE_VERBOSE=0
    ;;
    -r | --run)
      RUN_TARGET=1
    ;;
    -c | --cmake)
      RUN_CMAKE=1
    ;;
    esac
done

mkdir build
mkdir doc
cd build
rm bin/${TARGET}

if [ $RUN_CMAKE = 1 ]; then
  echo "RUNNING CMAKE"
  cmake ..
fi

make VERBOSE=$MAKE_VERBOSE
cp bin/$TARGET ../.

if [ $RUN_TARGET = 1 ]; then
   echo "RUNNING TARGET"
  ./bin/$TARGET
fi
