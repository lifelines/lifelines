#!/bin/sh
# test1
# Test gengedcomstrong and key preservation on a small, sparse db

pgm=$1
if ["$1" = ""]
then
pgm=../bin/llines
fi
# remove any leftovers
rm -rf qtest
$pgm -y qtest <<'End of input'
yurtest1.ged
y
rtest1.ll
1012
itest1.log
qqq
End of input
if diff test1.out test1.log > discrepancies
then
echo Success
else
echo See discrepancies file
fi

