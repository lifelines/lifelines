# A barebones autogen
# Created 2002.06.04
# A nicer one would test for presence of automake & autoconf first

aclocal -I m4 -I build/autotools
autoheader
automake --add-missing
autoconf

