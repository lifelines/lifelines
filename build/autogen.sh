#!/bin/sh

# autogen.sh - handy script to run all GNU autotools 

INCLUDE="-I build/autotools -I build/gettext"

cd ..

echo "Running aclocal..."
aclocal $INCLUDE

echo "Running autoheader..."
autoheader $INCLUDE

echo "Running automake..."
automake --add-missing

echo "Running autoconf..."
autoconf $INCLUDE

