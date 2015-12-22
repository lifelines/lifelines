#!/bin/sh

# Determine root of repository
if [ ! -f build_dist.sh ]
then
  if [ ! -f ../build_dist.sh ]
  then
    echo "ERROR: Must be run from either the root of the source tree or the build/ directory!"
    exit 1
  else
    ROOTDIR=..
  fi
else
  ROOTDIR=.
fi

UNAME=`uname`
if [ $UNAME != "Linux" ]
then
  echo "ERROR: Release tools only work on Linux due to bash-isms."
  exit 1
fi

SAVEDIR=`pwd`
cd $ROOTDIR

read -p 'Press key to fetch latest code from git:'

echo "STATUS: Updating local git repository..."
git checkout master
git pull

read -p 'Enter new version number (format X.Y.Z): ' newversion

echo ${newversion} | grep -E '^[[:digit:]]{1,2}\.[[:digit:]]{1,2}\.[[:digit:]]{1,2}$' > /dev/null
if [ $? -eq 0 ]
then
  continue 
else
  echo 'ERROR: Malformed version string.  Exiting.'
  exit 1
fi

echo "STATUS: Changing version number to $newversion..."
$ROOTDIR/build/setversions.sh $newversion

read -p 'Press key to run autotools:'

echo "STATUS: Generating new makefiles and configure script..."
$ROOTDIR/build/autogen.sh

read -p 'Press key to recreate local build subdirectory:'

echo "STATUS: Creating local build subdirectory..."
rm -rf bld
mkdir bld

read -p 'Press key to run configure:'

echo "STATUS: Running ./configure..."
cd bld
../configure

read -p 'Press key to build:'

echo "STATUS: Building..."
cd ..
make

read -p 'Press key to build new language files:'

echo "STATUS: Building language files..."
mv po/lifelines.pot po/lifelines.pot.old
cd po
make lifelines.pot
cd ..

read -p 'Press key to build distribution tarball:'

echo "STATUS: Creating distribution tarball..."
make dist
cd ..

echo "STATUS: Almost done..."
echo "You must read docs/dev/README.MAINTAINERS and finish the rest of the process!"
echo "Things to do include:"
echo "- tagging repo at github"
echo "- creating a release at github and uploading release packages"
echo "- sending message files to the translation project and updating message catalogs"
echo "- testing"

cd $SAVEDIR
