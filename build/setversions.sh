#!/bin/bash
# Perry Rapp, 2005-10-08
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
  sed $1 -e "$2" > $1
}

# Main function
function applyversion {
  SEDPAT="1s/^\(LifeLines Source Release, Version \)[[:alnum:].\-]*$/\1$VERSION/" 
  alterfile ../NEWS "$SEDPAT"
  alterfile ../INSTALL "$SEDPAT"
  alterfile ../README "$SEDPAT"
  SEDPAT="s/^\(AM_INIT_AUTOMAKE(lifelines, \)[[:alnum:].\-]*)$/\1$VERSION)/"
  alterfile ../configure.in "$SEDPAT"

#TODO - rest of files listed in README.MAINTAINERS
}

# Restore, for user to reverse last application
function restore {
  cp NEWS.bak NEWS
}

# Invoke whichever functionality was requested
if [ -z "RESTORE" ]
then
  restore
else
  applyversion
fi

