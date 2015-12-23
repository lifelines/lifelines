#!/bin/sh

# autogen.sh - handy script to run all GNU autotools 

INCLUDE="-I build/autotools -I build/gettext"

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

echo "Cleaning up old files..."
rm -f aclocal.m4
rm -rf autom4te.cache
rm -f config.*
rm -f configure
rm -f stamp-h1

echo "Running aclocal..."
aclocal $INCLUDE

echo "Running autoheader..."
autoheader $INCLUDE

echo "Running automake..."
automake --add-missing

echo "Running autoconf..."
autoconf $INCLUDE

cd $SAVEDIR
