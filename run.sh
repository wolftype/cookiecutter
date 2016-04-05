#!/bin/bash

# If run without arguments, this script
# will compile test/ccquote.cpp to build/bin and run it

# If another .cpp file in the test/ folder is passed as an argument:
#
#     ./build.sh test.cpp [-b] [-q]
#
# then it will build it to build/bin with the following options:
#
#      -b: build only, do not run
#      -v: make verbose


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

fi

echo Directory is $DIRECTORY
echo Filename is $FILENAME
echo Target is $TARGET

for i
  do
    case $i in
    -v | --verbose)
      MAKE_VERBOSE=1
    ;;
    -b | --build)
      RUN_TARGET=0
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


if [ $RUN_TARGET = 1 ]; then
   echo "RUNNING TARGET"
  ./bin/$TARGET
fi
