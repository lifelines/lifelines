#!/bin/sh

# This script will build and analyze the LifeLIines source code using the
# CScout program.  See http://www.spinellis.gr/cscout/

# Determine root of repository
if [ ! -f configure.ac ]
then
  if [ ! -f ../configure.ac ]
  then
    echo "ERROR: Must be run from either the root of the source tree or the build/ directory!"
    exit 1
  else
    ROOTDIR=..
  fi
else
  ROOTDIR=.
fi

SAVEDIR=`pwd`

cd $ROOTDIR

build/autogen.sh
./configure
csmake
cscout make.cs
# goes into daemon mode until killed
rm make.cs

cd $SAVEDIR
