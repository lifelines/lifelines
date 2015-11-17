#!/bin/sh

if [ ! -e "build_dist.sh" ]
then
 echo 'ERROR: This script must be run from the build/ directory.'
 exit 1
fi

read -p 'Press key to fetch latest code from git:'

echo "STATUS: Updating local git repository..."
git checkout master
git pull

read -p 'Enter new version number: ' newversion

echo ${newversion} | grep -E '^[[:digit:]]{1,2}\.[[:digit:]]{1,2}\.[[:digit:]]{1,2}$' > /dev/null

if [ $? -eq 0 ]
then
  continue 
else
  echo 'ERROR: Malformed version string.  Exiting.'
  exit 1
fi

echo "STATUS: Changing version number to $newversion..."
sh setversions.sh $newversion

read -p 'Press key to run autotools:'

echo "STATUS: Generating new makefiles and configure script..."
sh autogen.sh

read -p 'Press key to recreate local build subdirectory:'

echo "STATUS: Creating local build subdirectory..."
rm -rf bld
mkdir bld

read -p 'Press key to run configure:

echo "STATUS: Running ./configure..."
cd bld
../configure

read -p 'Press key to build:'

echo "STATUS: Building..."
cd ..
make

read -p 'Press key to build distribution tarball:'

echo "STATUS: Creating distribution tarball..."
make dist
cd ..

echo "STATUS: Almost done..."
echo "You must read docs/dev/README.MAINTAINERS and finish the rest of the process!"

