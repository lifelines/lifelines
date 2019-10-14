proc iftest1(v)
{
if (m, gt(v,4)) { "greater than 4" }
elsif (m, gt(v,3)) { "greater than 3" }
elsif (m, gt(v,2)) { "greater than 2" }
elsif (m, gt(v,1)) { "greater than 1" }
elsif (m, gt(v,0)) { "greater than 0" }
elsif (m, eq(v,0)) { "zero" }
else { "negative" }
}

proc iftest2(v)
{
if (m, gt(v,4)) { "greater than 4" }
elsif (m, gt(v,3)) { "greater than 3" }
elsif (m, gt(v,2)) { "greater than 2" }
elsif (m, gt(v,1)) { "greater than 1" }
elsif (m, gt(v,0)) { "greater than 0" }
elsif (m, eq(v,0)) { "zero" }
else { "negative" }
}

proc main()
{
  call iftest1(3)
  call iftest2(3)
}
