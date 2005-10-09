#!/bin/bash
# Created: 2005-10-08, Perry Rapp
# Edited:  2005-10-09, Perry Rapp
# Apply new version number to relevant lifelines source files
# Invoke this script like so:
#  sh setversions.sh 3.0.99

function showusage {
  script_parameters="3.0.xx (desired version number)"
  echo "Usage: `basename $0` $script_parameters"
}

# Check that user passed exactly one parameter
E_WRONG_ARGS=65
if [ $# -ne 1 ] || [ -z "$1" ]
then
  showusage
  exit $E_WRONG_ARGS
fi

# Function to handle parsing argument
function checkparm {
  if [ $1 = "restore" ]
  then
    RESTORE=1
    return
  fi

  # Store argument as $VERSION and check it is valid version
  VERSION=$1
  VPATTERN="^3\.0\.[[:alnum:]][[:alnum:]\.\-]*$"
  if ! echo $VERSION | grep -q $VPATTERN
  then
    showusage
    exit $E_WRONG_ARGS
  fi
}

function failexit {
  echo $1
  exit 99
}

# Parse argument (should be a version, or "restore")
checkparm $1

# Function to apply version to one file
# First argument is filename (eg, "NEWS")
# Second argument is sed pattern
function alterfile {
  cp $1 $1.bak || failexit "Error backing up file "$1
#TODO: Should fail if we don't get exactly one match in sed
  sed $1 -e "$2" > $1.tmp
  mv $1.tmp $1
}

# Function to restore file from last backup
function restorefile {
  if [ -e $1.bak ]
  then
    cp $1.bak $1
  fi
}

# Fix versions in an MS-Windows resource (rc) file
function alterwinversions {
  FILEPATH=$1
  cp $1 $1.bak2
  SEDPAT="s/\( FILEVERSION \)[0-9][0-9]*,[0-9][0-9]*,[0-9][0-9]*,[0-9][0-9]*/\1$VERSION1,$VERSION2,$VERSION3,$JULIANVERSION/"
  alterfile $FILEPATH "$SEDPAT"
  SEDPAT="s/\( PRODUCTVERSION \)[0-9][0-9]*,[0-9][0-9]*,[0-9][0-9]*,[0-9][0-9]*/\1$VERSION1,$VERSION2,$VERSION3,$JULIANVERSION/"
  alterfile $FILEPATH "$SEDPAT"
  SEDPAT="s/\([ ]*VALUE \"FileVersion\", \"\)[0-9][0-9]*, [0-9][0-9]*, [0-9][0-9]*, [0-9][0-9]*/\1$VERSION1, $VERSION2, $VERSION3, $JULIANVERSION/"
  alterfile $FILEPATH "$SEDPAT"
  SEDPAT="s/\([ ]*VALUE \"ProductVersion\", \"\)[0-9][0-9]*, [0-9][0-9]*, [0-9][0-9]*, [0-9][0-9]*/\1$VERSION1, $VERSION2, $VERSION3, $JULIANVERSION/"
  alterfile $FILEPATH "$SEDPAT"
  mv $1.bak2 $1.bak
}

# Main function
function applyversion {

  # Compute some numbers needed later

  YEAR=`date +%Y`
  MONTH=`date +%m`
  DAY=`date +%d`
  JULIANDAY=`date +%j`
  ((JULIANVERSION=($YEAR-1990)*1000+$JULIANDAY))
  if [[ $VERSION =~ '([0-9]+)\.([0-9]+)\.([0-9]+)' ]]
  then
    # numeric portion of version is now $BASH_REMATCH
    # eg, for 3.0.18p3-4, $BASH_REMATCH holds 3.0.18
    # and here we pick out the 3, 0, and 18
    VERSION1=${BASH_REMATCH[1]}
    VERSION2=${BASH_REMATCH[2]}
    VERSION3=${BASH_REMATCH[3]}
  else
    failexit "Version could not be parsed into expected format"
  fi
 echo "Full version=<$VERSION>"
 echo "Numeric version=<$VERSION1.$VERSION2.$VERSION3>"
 echo "Windows version=<$VERSION1.$VERSION2.$VERSION3.$JULIANVERSION>"

  # Now do version substitutions in each file
  # Call alterfile with an sed command for each file

  SEDPAT="1s/^\(LifeLines Source Release, Version \)[[:alnum:].\-]*$/\1$VERSION/" 
  alterfile ../NEWS "$SEDPAT"
  alterfile ../INSTALL "$SEDPAT"
  alterfile ../README "$SEDPAT"
  SEDPAT="s/^\(AM_INIT_AUTOMAKE(lifelines, \)[[:alnum:].\-]*)$/\1$VERSION)/"
  alterfile ../configure.in "$SEDPAT"
  SEDPAT="s/\(%define lifelines_version [ ]*\)[[:alnum:].\-]*$/\1$VERSION/"
  alterfile ../build/rpm/lifelines.spec "$SEDPAT"
  SEDPAT="s/\(#define LIFELINES_VERSION \"\)[[:alnum:].\-]*\"$/\1$VERSION\\\"/"
  alterfile ../src/hdrs/version.h "$SEDPAT"

# TODO (remaining files from README.MAINTAINERS):
# docs/ll-devguide.xml (1 occurrence)
# docs/ll-reportmanual.xml (2 occurrences)
# docs/ll-userguide.xml (2 occurrences)
# docs/llines.1 (& year & month as well)

  alterwinversions ../build/msvc6/dbverify/dbVerify.rc
  alterwinversions ../build/msvc6/llexec/llexec.rc
  alterwinversions ../build/msvc6/llines/llines.rc
}

# Restore, for user to reverse last application
function restore {
  restorefile ../NEWS
  restorefile ../INSTALL
  restorefile ../README
  restorefile ../configure.in
  restorefile ../build/rpm/lifelines.spec
  restorefile ../src/hdrs/version.h

  restorefile ../build/msvc6/dbverify/dbVerify.rc
  restorefile ../build/msvc6/llexec/llexec.rc
  restorefile ../build/msvc6/llines/llines.rc
}

# Invoke whichever functionality was requested
if [ -z "$RESTORE" ]
then
  applyversion
else
  restore
fi

