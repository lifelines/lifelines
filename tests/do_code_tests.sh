#!/bin/sh

# test programs
PROGS=../src/tools/lltest
LLINES=../src/liflines/llines
DBNAME=testdb

# counters
TOTALCNT=0
FAILCNT=0
FAILED=""

if [ ! -f $LLINES ]
then
  echo "ERROR: Could not find lifelines!"
  exit
fi

# execute all test programs
for i in $PROGS
do
  TESTDIR=`dirname $i`
  TESTSRC=`basename $i`
  OUTFILE=${TESTSRC}.out
  echo "Running program tests via $i..."

  if [ ! -f $i ]
  then
    echo "ERROR: Could not find test $i"
    exit
  fi

  # remove test database
  rm -rf $DBNAME

  # create empty database
  echo "yq" | $LLINES $DBNAME

  # run test program
  $i $DBNAME > ${TESTSRC}.out

  # remove test database
  rm -rf $DBNAME

  TOTALCNT=`wc -l ${OUTFILE}`
  FAILCNT=`grep FAIL ${OUTFILE} | wc -l`
done

# print summary
echo
echo "Executed $TOTALCNT tests, with $FAILCNT failures."
if [ $FAILCNT -gt 0 ]
then
  echo $FAILED
fi
