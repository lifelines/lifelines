#
# lifelines.spec - RPM configuration file for Lifelines
#
# To generate RPMs, place the tar.gz in /usr/src/packages/SOURCES/ and
# then "rpm -ba lifelines.spec"
#

%define lifelines_version       3.0.5

Name: lifelines
Summary: lifelines genealogy program
Version: %{lifelines_version}
Release: 1
Copyright: MIT
Group: Utilities/System
Source:         http://download.sourceforge.net/lifelines/lifelines-%{lifelines_version}.tar.gz
URL:            http://lifelines.sourceforge.net/
Packager:       Marc Nozell <marc@nozell.com>
Provides:       lifelines
%description
This program allows the tracking of genealogical information.  The lifelines
reports are the power of the system.

%prep 
%setup

%build
#make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"
sh -c ./configure 
make 

%install
install -s -m 755 -o 0 -g 0 liflines/llines /usr/local/bin/llines
#install -m 644 -o 0 -g 0 eject.1 /usr/man/man1

%files
%doc README ChangeLog ANNOUNCEMENT AUTHORS LICENSE

/usr/local/bin/llines

