#!/bin/sh

# This script will build and analyze the LifeLiines source code using the
# CScout program.  See http://www.spinellis.gr/cscout/

# Determine root of repository
if [ ! -f configure.ac ]
then
  echo "ERROR: Must be run from the root of the source tree!"
  exit 1
else
  ROOTDIR=`pwd`
fi

CSDIR=$ROOTDIR/build/cscout

# Create directory for cscout database
if [ ! -d $CSDIR ]
then
  mkdir $CSDIR
fi

# Run configure if needed
if [ ! -f config.h ]
then
  build/autogen.sh
  ./configure
fi

# Compile workspace
cswc build/lifelines.cscout > $CSDIR/lifelines.cs

# Run Web Daemon
cscout $CSDIR/lifelines.cs

# Clean up
rm $CSDIR/lifelines.cs

