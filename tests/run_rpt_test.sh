#!/bin/sh
#
# Generic Test Runner
#
# In directory $1 run test $2

testdir=$1
testscr=$2
testname=`echo $testscr | sed -e 's/.llscr$//g'`

if [ -z $testdir -o -z $testscr ]
then
  echo "Syntax: $0 <directory> <script>"
  exit
fi

if [ ! -d $testdir ]
then
  echo "ERROR: Directory $1 not found!"
  exit
fi

if [ ! -f "$testdir/$testscr" ]
then
  echo "ERROR: Script $testscr not found!"
  exit
fi

if [ "$testscr" = "$testname" ]
then
  echo "ERROR: Script $testscr is not a script (llscr) file!"
  exit
fi

testprog=`pwd`/../src/liflines/llines
origdir=`pwd`
 
# change to test directory
cd $testdir

# remove test database
rm -rf testdb

# run test
$testprog testdb < $testscr

# remove test database
rm -rf testdb

# check results
if diff $testname.log.save $testname.log > $testname.diff
then
  rm $testname.diff
  rm $testname.log
  echo Success
else
  echo Failure, see $testname.diff and $testname.log
fi

# change to original directory
cd $origdir
