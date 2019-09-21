#!/bin/bash
# Created: 2005-10-08, Perry Rapp
# Edited:  2007-05-12, Perry Rapp
# Edited:  2015-12-22, Matt Emmerton
#
# Apply new version number to relevant lifelines source files
# Invoke this script like so:
#  sh setversions.sh 3.0.99

##
## SUBROUTINES
##

function showusage {
  echo "Usage: sh `basename $0` X.Y.Z        # to change to specified version number"
  echo "   or: sh `basename $0` TAG          # to change to specific version tag"
  echo "   or: sh `basename $0` restore      # to undo version number just applied"
  echo "   or: sh `basename $0` cleanup      # to remove backup files"
}

function usageexit {
  echo $1
  showusage
  exit $2
}

function failexit {
  echo $1
  exit 99
}

function checkparm {
  if [ $1 = "restore" ]
  then
    RESTORE=1
    return
  fi

  if [ $1 = "cleanup" ]
  then
    CLEANUP=1
    return
  fi

  # Check for valid version
  VERSIONFOUND=0
  VPATTERN="^[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*$"
  if echo $1 | grep -q $VPATTERN
  then
    VERSION=$1
    VERSIONFOUND=1
  fi

  # Check for valid tag
  TAGFOUND=0
  TPATTERN="^(alpha|beta|rc[1-9][0-9]*|release)$"
  if echo $1 | grep -qE $TPATTERN
  then
    TAG=$1
    TAGFOUND=1
  fi

  if ( [ $VERSIONFOUND -eq 0 ] && [ $TAGFOUND -eq 0 ] )
  then
    usageexit "ERROR: Missing or invalid version number or tag" $E_WRONG_ARGS
  fi
}

# Function to apply version to one file
# Argument#1 is filename (eg, "NEWS")
# Arguments#2-N sed patterns, applied one after another
function alterfile {
  [ ! -z "$1" ] || failexit "Missing first argument to alterfile"
  [ ! -z "$2" ] || failexit "Missing second argument to alterfile"
  FILEPATH=$1
  shift
  cp $FILEPATH $FILEPATH.bak || failexit "Error backing up file "$FILEPATH
  # Now apply each remaining argument as sed command
  echo "Updating $FILEPATH..."
  until [ -z "$1" ]
  do
    # sed doesn't seem to set its return value, so we don't check
    # that it found its match, unfortunately
    sed -e "$1" $FILEPATH > $FILEPATH.tmp
    mv $FILEPATH.tmp $FILEPATH || failexit "mv failed in alterfile: $FILEPATH.tmp"
    shift
  done
}

function altermansrc {
  [ ! -z "$1" ] || failexit "Missing first argument to altermansrc"
  [ ! -z "$2" ] || failexit "Missing first argument to altermansrc"
  MANFILE=$1
  PROGNAME=$2
  SEDPAT="s/\(^\.TH $PROGNAME 1 \"\)[0-9]\{4\}/\1$YEAR/"
  SEDPAT2="s/\(^\.TH $PROGNAME 1 \"$YEAR \)[[:alpha:]]\{3\}/\1$MONTHABBR/"
  SEDPAT3="s/\(^\.TH $PROGNAME 1 \"$YEAR $MONTHABBR\" \"Lifelines \)[0-9][[:alnum:].\-]*/\1$VERSION/"
  alterfile $MANFILE "$SEDPAT" "$SEDPAT2" "$SEDPAT3"
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

# Function to restore file from last backup
# Argument#1: file to restore (from .bak version)
function restorefile {
  if [ -e $1.bak ]
  then
    cp $1.bak $1
  fi
}

# Function to remove backup files
# Argument#1: file to remove
function cleanupfile {
  if [ -e $1.bak ]
  then
    rm $1.bak
  fi
}

# Extract and compute new version numbers
function getversion {
  YEAR=`date +%Y`
  MONTHABBR=`date +%b`
  YMD=`date +%Y-%m-%d`
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
}

# Call alterfile with an sed command for each file
function applyversion {
  SEDPAT="s/^\(LifeLines Source Release, Version \)[[:alnum:].\-]*$/\1$VERSION/" 
  alterfile $ROOTDIR/AUTHORS "$SEDPAT"
  alterfile $ROOTDIR/ChangeLog "$SEDPAT"
  alterfile $ROOTDIR/INSTALL "$SEDPAT"
  alterfile $ROOTDIR/NEWS "$SEDPAT"
  alterfile $ROOTDIR/README "$SEDPAT"

  SEDPAT="s/\(AC_INIT(lifelines,[ ]*\)[0-9][[:alnum:].\-]*)$/\1$VERSION)/"
  alterfile $ROOTDIR/configure.ac "$SEDPAT"

  SEDPAT="s/\(%define lifelines_version [ ]*\)[0-9][[:alnum:].\-]*$/\1$VERSION/"
  alterfile $ROOTDIR/build/rpm/lifelines.spec "$SEDPAT"

  SEDPAT="s/\(release version=\)\"[0-9][[:alnum:].\-]*\"/\1\"$VERSION\"/"
  SEDPAT2="s/\(release.*date=\)\"[0-9]*-[0-9]*-[0-9]*\"/\1\"$YMD\"/"
  alterfile $ROOTDIR/build/appdata/lifelines.appdata.xml "$SEDPAT" "$SEDPAT2"

  alterwinversions $ROOTDIR/build/msvc6/btedit/btedit.rc
  alterwinversions $ROOTDIR/build/msvc6/dbverify/dbVerify.rc
  alterwinversions $ROOTDIR/build/msvc6/llexec/llexec.rc
  alterwinversions $ROOTDIR/build/msvc6/llines/llines.rc

  altermansrc $ROOTDIR/docs/man/btedit.1 btedit
  altermansrc $ROOTDIR/docs/man/dbverify.1 dbverify
  altermansrc $ROOTDIR/docs/man/llines.1 llines
  altermansrc $ROOTDIR/docs/man/llexec.1 llexec

  SEDPAT="s/\(<\!ENTITY llversion[[:space:]]*['\"']\)[0-9][[:alnum:].\-]*/\1$VERSION/i"
  alterfile $ROOTDIR/docs/manual/ll-devguide.xml "$SEDPAT"
  alterfile $ROOTDIR/docs/manual/ll-userguide.xml "$SEDPAT"
  alterfile $ROOTDIR/docs/manual/ll-userguide.sv.xml "$SEDPAT"
  alterfile $ROOTDIR/docs/manual/ll-reportmanual.xml "$SEDPAT"
  alterfile $ROOTDIR/docs/manual/ll-reportmanual.sv.xml "$SEDPAT"
}

# Call alterfile with an sed command for each file
function applytag {
  SEDPAT="s/^\(#define LIFELINES_VERSION_EXTRA \)\"([[:alnum:]]*)\"$/\1\"($TAG)\"/"
  alterfile $ROOTDIR/src/hdrs/version.h "$SEDPAT"
}

# Restore, for user to reverse last application
function restore {
  restorefile $ROOTDIR/AUTHORS
  restorefile $ROOTDIR/ChangeLog
  restorefile $ROOTDIR/INSTALL
  restorefile $ROOTDIR/NEWS
  restorefile $ROOTDIR/README
  restorefile $ROOTDIR/configure.ac
  restorefile $ROOTDIR/build/appdata/lifelines.appdata.xml
  restorefile $ROOTDIR/build/msvc6/btedit/btedit.rc
  restorefile $ROOTDIR/build/msvc6/dbverify/dbVerify.rc
  restorefile $ROOTDIR/build/msvc6/llexec/llexec.rc
  restorefile $ROOTDIR/build/msvc6/llines/llines.rc
  restorefile $ROOTDIR/build/rpm/lifelines.spec
  restorefile $ROOTDIR/docs/man/btedit.1
  restorefile $ROOTDIR/docs/man/dbverify.1
  restorefile $ROOTDIR/docs/man/llines.1
  restorefile $ROOTDIR/docs/man/llexec.1
  restorefile $ROOTDIR/docs/manual/ll-devguide.xml
  restorefile $ROOTDIR/docs/manual/ll-reportmanual.xml
  restorefile $ROOTDIR/docs/manual/ll-reportmanual.sv.xml
  restorefile $ROOTDIR/docs/manual/ll-userguide.xml
  restorefile $ROOTDIR/docs/manual/ll-userguide.sv.xml
  restorefile $ROOTDIR/src/hdrs/version.h
}

# Cleanup, for user to remove backup files
function cleanup {
  cleanupfile $ROOTDIR/AUTHORS
  cleanupfile $ROOTDIR/ChangeLog
  cleanupfile $ROOTDIR/INSTALL
  cleanupfile $ROOTDIR/NEWS
  cleanupfile $ROOTDIR/README
  cleanupfile $ROOTDIR/configure.ac
  cleanupfile $ROOTDIR/build/appdata/lifelines.appdata.xml
  cleanupfile $ROOTDIR/build/msvc6/btedit/btedit.rc
  cleanupfile $ROOTDIR/build/msvc6/dbverify/dbVerify.rc
  cleanupfile $ROOTDIR/build/msvc6/llexec/llexec.rc
  cleanupfile $ROOTDIR/build/msvc6/llines/llines.rc
  cleanupfile $ROOTDIR/build/rpm/lifelines.spec
  cleanupfile $ROOTDIR/docs/man/btedit.1
  cleanupfile $ROOTDIR/docs/man/dbverify.1
  cleanupfile $ROOTDIR/docs/man/llines.1
  cleanupfile $ROOTDIR/docs/man/llexec.1
  cleanupfile $ROOTDIR/docs/manual/ll-devguide.xml
  cleanupfile $ROOTDIR/docs/manual/ll-reportmanual.xml
  cleanupfile $ROOTDIR/docs/manual/ll-reportmanual.sv.xml
  cleanupfile $ROOTDIR/docs/manual/ll-userguide.xml
  cleanupfile $ROOTDIR/docs/manual/ll-userguide.sv.xml
  cleanupfile $ROOTDIR/src/hdrs/version.h
}

##
## MAIN PROGRAM
##

# Determine root of repository
if [ ! -f ChangeLog ]
then
  if [ ! -f ../ChangeLog ]
  then
    failexit "ERROR: Must be run from either the root of the source tree or the build/ directory!"
  else
    ROOTDIR=..
  fi
else
  ROOTDIR=.
fi

# Check that user passed exactly one parameter
E_WRONG_ARGS=65
if ( [ $# -ne 1 ] && [ $# -ne 2 ] ) || [ -z "$1" ]
then
  usageexit "ERROR: Wrong number of arguments!" $E_WRONG_ARGS
fi

# Function to handle parsing argument
# Parse argument (should be a version, or "restore" or "cleanup")
# (exits if failure)
checkparm $1 $2

# Invoke whichever functionality was requested
if [ ! -z "$RESTORE" ]
then
  restore
elif [ ! -z "$CLEANUP" ]
then
  cleanup
else
  if [ $VERSIONFOUND -eq 1 ]
  then
    echo "Applying version change..."
    getversion
    applyversion
  fi

  if [ $TAGFOUND -eq 1 ] 
  then
    echo "Appyling tag change..."
    applytag
  fi
fi 
