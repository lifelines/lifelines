#!/bin/bash
# Created: 2005-10-08, Perry Rapp
# Edited:  2007-04-29, Perry Rapp
# Apply new version number to relevant lifelines source files
# Invoke this script like so:
#  sh setversions.sh 3.0.99

function showusage {
  script_parameters="3.0.xx (desired version number)"
  echo "Usage: sh `basename $0` $script_parameters"
  echo "  or: sh `basename $0` restore (to undo versions just applied"
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
# (exits if failure)
checkparm $1

# Function to apply version to one file
# Argument#1 is filename (eg, "NEWS")
# Arguments#2-? sed patterns, applied one after another
function alterfile {
  [ ! -z "$1" ] || failexit "Missing first argument to alterfile"
  [ ! -z "$2" ] || failexit "Missing second argument to alterfile"
  FILEPATH=$1
  shift
  cp $FILEPATH $FILEPATH.bak || failexit "Error backing up file "$FILEPATH
  # Now apply each remaining argument as sed command
  until [ -z "$1" ]
  do
    # sed doesn't seem to set its return value, so we don't check
    # that it found its match, unfortunately
    sed $FILEPATH -e "$1" > $FILEPATH.tmp
    mv $FILEPATH.tmp $FILEPATH || failexit "mv failed in alterfile: $FILEPATH.tmp"
    shift
  done
}

# Function to restore file from last backup
# Argument#1: file to restore (from .bak version)
function restorefile {
  if [ -e $1.bak ]
  then
    cp $1.bak $1
  fi
}

# Fix versions in an MS-Windows resource (rc) file
# Argument#1: file to change
function alterwinversions {
  FILEPATH=$1
  SEDPAT1="s/\( FILEVERSION \)[0-9][0-9]*,[0-9][0-9]*,[0-9][0-9]*,[0-9][0-9]*/\1$VERSION1,$VERSION2,$VERSION3,$JULIANVERSION/"
  SEDPAT2="s/\( PRODUCTVERSION \)[0-9][0-9]*,[0-9][0-9]*,[0-9][0-9]*,[0-9][0-9]*/\1$VERSION1,$VERSION2,$VERSION3,$JULIANVERSION/"
  SEDPAT3="s/\([ ]*VALUE \"FileVersion\", \"\)[0-9][0-9]*, [0-9][0-9]*, [0-9][0-9]*, [0-9][0-9]*/\1$VERSION1, $VERSION2, $VERSION3, $JULIANVERSION/"
  SEDPAT4="s/\([ ]*VALUE \"ProductVersion\", \"\)[0-9][0-9]*, [0-9][0-9]*, [0-9][0-9]*, [0-9][0-9]*/\1$VERSION1, $VERSION2, $VERSION3, $JULIANVERSION/"
  alterfile $FILEPATH "$SEDPAT1" "$SEDPAT2" "$SEDPAT3" "$SEDPAT4"
}

# Main function
function applyversion {

  # Compute some numbers needed later

  YEAR=`date +%Y`
  MONTHABBR=`date +%b`
  DAY=`date +%d`
  JULIANDAY=`date +%-j`
  JULIANVERSION=$((($YEAR-1990)*1000 + $JULIANDAY))
  if [[ $VERSION =~ ([0-9]+)\.([0-9]+)\.([0-9]+) ]]
  then
    # numeric portion of version is now $BASH_REMATCH
    # eg, for 3.0.18p3-4, $BASH_REMATCH holds 3.0.18
    # and here we pick out the 3, 0, and 18
    VERSION1=${BASH_REMATCH[1]}
    VERSION2=${BASH_REMATCH[2]}
    VERSION3=${BASH_REMATCH[3]}
  else
    failexit "Version ($VERSION) could not be parsed into expected format"
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
  SEDPAT="s/^\(AM_INIT_AUTOMAKE(lifelines, \)[0-9][[:alnum:].\-]*)$/\1$VERSION)/"
  alterfile ../configure.in "$SEDPAT"
  SEDPAT="s/\(%define lifelines_version [ ]*\)[0-9][[:alnum:].\-]*$/\1$VERSION/"
  alterfile ../build/rpm/lifelines.spec "$SEDPAT"
  SEDPAT="s/\(#define LIFELINES_VERSION \"\)[0-9][[:alnum:].\-]*\"$/\1$VERSION\\\"/"
  alterfile ../src/hdrs/version.h "$SEDPAT"
  SEDPAT="s/\(\[<!ENTITY llversion '\)[0-9][[:alnum:].\-]*/\1$VERSION/"
  alterfile ../docs/ll-devguide.xml "$SEDPAT"
  SEDPAT2="s/\(<\!entity llversion[[:space:]]*\"\)[0-9][[:alnum:].\-]*/\1$VERSION/"
  alterfile ../docs/ll-reportmanual.xml "$SEDPAT" "$SEDPAT2"
  alterfile ../docs/ll-userguide.xml "$SEDPAT" "$SEDPAT2"
  altermansrc btedit.1 btedit
  altermansrc dbverify.1 dbverify
  altermansrc llines.1 llexec.1 LLINES
  alterwinversions ../build/msvc6/dbverify/dbVerify.rc
  alterwinversions ../build/msvc6/llexec/llexec.rc
  alterwinversions ../build/msvc6/llines/llines.rc
}

function altermansrc {
  [ ! -z "$1" ] || failexit "Missing first argument to altermansrc"
  [ ! -z "$2" ] || failexit "Missing first argument to altermansrc"
  MANFILE=$1
  PROGNAME=$2
  SEDPAT="s/\(^\.TH $PROGNAME 1 \"\)[0-9]\{4\}/\1$YEAR/"
  SEDPAT2="s/\(^\.TH $PROGNAME 1 \"$YEAR \)[[:alpha:]]\{3\}/\1$MONTHABBR/"
  SEDPAT3="s/\(^\.TH $PROGNAME 1 \"$YEAR $MONTHABBR\" \"Lifelines \)[0-9][[:alnum:].\-]*/\1$VERSION/"
  alterfile ../docs/$MANFILE "$SEDPAT" "$SEDPAT2" "$SEDPAT3"
}

# Restore, for user to reverse last application
function restore {
  restorefile ../NEWS
  restorefile ../INSTALL
  restorefile ../README
  restorefile ../configure.in
  restorefile ../build/rpm/lifelines.spec
  restorefile ../src/hdrs/version.h
  restorefile ../docs/ll-devguide.xml
  restorefile ../docs/ll-reportmanual.xml
  restorefile ../docs/ll-userguide.xml
  restorefile ../docs/llines.1
  restorefile ../docs/llexec.1
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

