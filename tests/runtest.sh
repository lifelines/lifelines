#!/bin/sh
#
# Generic Test Runner
#
# Using $1, run test $2 in directory $3.

testprog=`pwd`/../src/liflines/llines
testdir=$1
testscr=$2
origdir=`pwd`
testname=`echo $testscr | sed -e 's/.llscr//g'`
 
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
