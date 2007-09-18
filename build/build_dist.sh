if [ ! -e "README.MAINTAINERS" ]
then
 echo 'Wrong Directory! Must be run in lifelines directory'
 exit 1
fi

read -p 'Press key to fetch latest cvs (cvs -z3 up -d)'

cvs -z3 up -d

read -p 'Enter new version number: ' inputvariable

echo ${inputvariable} | grep -E '^[[:digit:]]{1,2}\.[[:digit:]]{1,2}\.[[:digit:]]{1,2}$' > /dev/null

if [ $? -eq 0 ]; then
  continue 
else
  echo 'Malformed version string; exiting'
  exit 1
fi


# cd build; sh setversions.sh 3.0.19; cd ..

read -p 'Press key to run autotools (sh autogen.sh)'

sh autogen.sh

read -p 'Press key to remove & recreate bld subdir'
rm -rf bld
mkdir bld

read -p 'Press key to run configure'

cd bld
../configure

read -p 'Press key to compile (run make)'

make

read -p 'Press key to update master message catalog'

mv ../po/lifelines.pot ../po/lifelines.old.pot
cd po
make lifelines.pot
cd ..

read -p 'Press key to build distribution tarball'

make dist
cd ..

read -p 'Return to README.MAINTAINERS and finish directions'


