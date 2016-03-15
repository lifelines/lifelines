#!/bin/sh

function prompt {
  echo "----------------------------------------"
  echo $1
  echo "----------------------------------------"
  read -p "Press ENTER to continue..." 
}

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

prompt '1) Fetch latest code from git'

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

prompt '2) Change version number in documentation and build metadata'

sh build/setversions.sh $newversion

prompt '3) Run autotools'

sh build/autogen.sh

prompt '4) Build distribution copy of repo for release building'

# Why is this important?  We want anyone to be able to rebuild a release from
# a source-only tarball, which is what our releases are.  If we can't do this
# ourselves, how can we expect anyone else to do so?

./configure --with-docs
make distclean

prompt '5) Configure staging area for release'

rm -rf staging
mkdir staging
cd staging
../configure --with-docs

prompt '6) Build in staging area for release'

make

prompt '7) Build release tarball in staging area'

make dist

echo "The distribution tarball can be found here:"
ls -al staging/lifelines*gz
echo
echo "But there is more to be done!"
echo "You must read docs/dev/README.MAINTAINERS and finish the rest of the process!"
echo "Things to do (for pre-release) include:"
echo "- sending message files to the translation project and updating message catalogs"
echo "Things to do (for release) include:"
echo "1) Source
echo "- checking in changes due to version number changes"
echo "- changing in changes to language files due to source line number shifts"
echo "- cleaning up from the version number changes (sh setversions.sh cleanup)"
echo "2) Repository
echo "- tagging repo at github"
echo "- creating a release at github and uploading release packages"

cd $SAVEDIR
