#!/bin/sh

# autogen.sh - handy script to run all GNU autotools 

INCLUDE="-I build/autotools -I build/gettext"

if [ ! -f configure.ac ]
then
  echo "ERROR: Must be run from the directory containing configure.ac!"
  exit 1
fi

echo "Running aclocal..."
aclocal $INCLUDE

echo "Running autoheader..."
autoheader $INCLUDE

echo "Running automake..."
automake --add-missing

echo "Running autoconf..."
autoconf $INCLUDE

