#!/bin/sh

# find all test programs
PROGS=`find . -type f -name \*.llscr`

# counters
TOTALCNT=0
FAILCNT=0
FAILED=""

# execute all test programs
for i in $PROGS
do
  TESTDIR=`dirname $i`
  TESTSCR=`basename $i`
  TESTNAME=`echo $TESTSCR | sed -e 's/.llscr//g'`
  TOTALCNT=$((TOTALCNT+1))

  echo "Running test $i..."
  ./run_rpt_test.sh $TESTDIR $TESTSCR

  # If diff file exists and is non-empty, then test has failed.
  if [ -f $TESTDIR/$TESTNAME.diff -a -s $TESTDIR/$TESTNAME.diff ]
  then
    if [ -z $FAILED ]
    then
      FAILED=$i
    else
      FAILED="${FAILED},$i"   
    fi
    FAILCNT=$((FAILCNT+1))
  fi
done

# print summary
echo
echo "Executed $TOTALCNT tests, with $FAILCNT failures."
if [ $FAILCNT -gt 0 ]
then
  echo $FAILED
fi
