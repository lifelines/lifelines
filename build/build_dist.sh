#!/bin/sh

# Determine root of repository
if [ ! -f ChangeLog ]
then
  if [ ! -f ../ChangeLog ]
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
  echo "ERROR: Release tools only work on Linux due to dependency on bash and gnumake."
  exit 1
fi

SAVEDIR=`pwd`
cd $ROOTDIR

read -p 'Press key to fetch latest code from git:'

echo "STATUS: Updating local git repository..."
git checkout master
git pull

newversion=`grep AC_INIT configure.ac | sed -e 's/AC_INIT//g' -e 's/lifelines,//g' -e 's/[\(\), ]//g'`
echo "New version number, as parsed from configure.ac: $newversion"

echo ${newversion} | grep -E '^[[:digit:]]{1,2}\.[[:digit:]]{1,2}\.[[:digit:]]{1,2}$' > /dev/null
if [ $? -eq 0 ]
then
  continue 
else
  echo 'ERROR: Malformed version string.  Exiting.'
  exit 1
fi

read -p 'Press key to change version number across the distribution'

echo "STATUS: Changing version number to $newversion..."
sh $ROOTDIR/build/setversions.sh $newversion

read -p 'Press key to run autotools:'

echo "STATUS: Generating new makefiles and configure script..."
sh $ROOTDIR/build/autogen.sh

read -p 'Press key to clean local repository'

echo "STATUS: Cleaning local repository..."
./configure
make distclean

read -p 'Press key to recreate staging area for release build:'

echo "STATUS: Creating staging area for release build..."
rm -rf staging
mkdir staging

read -p 'Press key to run configure:'

echo "STATUS: Running ./configure..."
cd staging
../configure

read -p 'Press key to build:'

echo "STATUS: Building..."
make

read -p 'Press key to build distribution tarball:'

echo "STATUS: Creating distribution tarball..."
make dist

echo "STATUS: Almost done..."
echo "You must read docs/dev/README.MAINTAINERS and finish the rest of the process!"
echo "Things to do include:"
echo "- tagging repo at github"
echo "- creating a release at github and uploading release packages"
echo "- sending message files to the translation project and updating message catalogs"
echo "- testing"

cd $SAVEDIR
