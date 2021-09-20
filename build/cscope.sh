#!/bin/sh

# This script will build and use an index of LifeLines source code using the
# CScope program.  See http://cscope.sourceforge.net/.

# Determine root of repository
if [ ! -f configure.ac ]
then
  echo "ERROR: Must be run from the root of the source tree!"
  exit 1
else
  ROOTDIR=`pwd`
fi

CSDIR=$ROOTDIR/build/cscope

# Create directory for cscope database
if [ ! -d $CSDIR ]
then
  mkdir $CSDIR
fi

# Find files
find $ROOTDIR -name \*.c -o -name \*.h > $CSDIR/cscope.files

# Generate database
cd $CSDIR
cscope -b -q -k
cd $ROOTDIR

# Use database
cd $CSDIR
cscope -d
cd $ROOTDIR

