#
# lifelines.spec - RPM configuration file for Lifelines
#
# To generate RPMs, place the tar.gz in /usr/src/packages/SOURCES/ and
# (or /usr/src/redhat/SOURCES as appropriate)
# then "rpm -ba lifelines.spec"
# or on newer systems, "rpmbuild -ba lifelines.spec"
#
# Add comment to top of comments at bottom if you revise this file.
#

%define lifelines_version       3.0.62
%define reports_dir /usr/local/share/lifelines-%{lifelines_version}/reports
%define tt_dir /usr/local/share/lifelines-%{lifelines_version}/tt

Name: lifelines
Summary: lifelines genealogy program
Version: %{lifelines_version}
Release: 1
License: X11
Group: Utilities/System
Source:         http://download.sourceforge.net/lifelines/lifelines-%{lifelines_version}.tar.gz
URL:            http://lifelines.sourceforge.net/
Packager:       Marc Nozell <marc@nozell.com>
Provides:       lifelines
BuildRoot: %{_tmppath}/%{name}-%{version}-root
BuildRequires: ncurses-devel bison

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
install -d -m 755 $RPM_BUILD_ROOT/usr/local/bin
install -s -m 755 src/liflines/llines $RPM_BUILD_ROOT/usr/local/bin
install -s -m 755 src/liflines/llexec $RPM_BUILD_ROOT/usr/local/bin
install -s -m 755 src/tools/dbverify $RPM_BUILD_ROOT/usr/local/bin

install -d -m 755 $RPM_BUILD_ROOT/usr/local/share/locale/da/LC_MESSAGES
install -m 644 po/da.gmo $RPM_BUILD_ROOT/usr/local/share/locale/da/LC_MESSAGES/lifelines.mo

install -d -m 755 $RPM_BUILD_ROOT/usr/local/share/locale/de/LC_MESSAGES
install -m 644 po/de.gmo $RPM_BUILD_ROOT/usr/local/share/locale/de/LC_MESSAGES/lifelines.mo

install -d -m 755 $RPM_BUILD_ROOT/usr/local/share/locale/eo/LC_MESSAGES
install -m 644 po/eo.gmo $RPM_BUILD_ROOT/usr/local/share/locale/eo/LC_MESSAGES/lifelines.mo

install -d -m 755 $RPM_BUILD_ROOT/usr/local/share/locale/es/LC_MESSAGES
install -m 644 po/es.gmo $RPM_BUILD_ROOT/usr/local/share/locale/es/LC_MESSAGES/lifelines.mo

install -d -m 755 $RPM_BUILD_ROOT/usr/local/share/locale/fr/LC_MESSAGES
install -m 644 po/fr.gmo $RPM_BUILD_ROOT/usr/local/share/locale/fr/LC_MESSAGES/lifelines.mo

install -d -m 755 $RPM_BUILD_ROOT/usr/local/share/locale/nl/LC_MESSAGES
install -m 644 po/nl.gmo $RPM_BUILD_ROOT/usr/local/share/locale/nl/LC_MESSAGES/lifelines.mo

install -d -m 755 $RPM_BUILD_ROOT/usr/local/share/locale/pl/LC_MESSAGES
install -m 644 po/pl.gmo $RPM_BUILD_ROOT/usr/local/share/locale/pl/LC_MESSAGES/lifelines.mo

install -d -m 755 $RPM_BUILD_ROOT/usr/local/share/locale/rw/LC_MESSAGES
install -m 644 po/rw.gmo $RPM_BUILD_ROOT/usr/local/share/locale/rw/LC_MESSAGES/lifelines.mo

install -d -m 755 $RPM_BUILD_ROOT/usr/local/share/locale/sv/LC_MESSAGES
install -m 644 po/sv.gmo $RPM_BUILD_ROOT/usr/local/share/locale/sv/LC_MESSAGES/lifelines.mo

install -d -m 755 $RPM_BUILD_ROOT%{reports_dir}
install -m 644 reports/*.ll $RPM_BUILD_ROOT%{reports_dir}
install -m 644 reports/*.li $RPM_BUILD_ROOT%{reports_dir}
install -m 644 reports/CREDIT reports/index.html $RPM_BUILD_ROOT%{reports_dir}
install -m 644 reports/boc.gif reports/ll.png $RPM_BUILD_ROOT%{reports_dir}
install -m 644 reports/*.c $RPM_BUILD_ROOT%{reports_dir}
install -m 644 reports/ps-pedigree.ps reports/tree.tex $RPM_BUILD_ROOT%{reports_dir}

install -d -m 755 $RPM_BUILD_ROOT%{reports_dir}/novel
install -m 644 reports/novel/novel* $RPM_BUILD_ROOT%{reports_dir}/novel

install -d -m 755 $RPM_BUILD_ROOT%{reports_dir}/pedtex
install -m 644 reports/pedtex/pedtex* $RPM_BUILD_ROOT%{reports_dir}/pedtex
install -m 644 reports/pedtex/*.tex $RPM_BUILD_ROOT%{reports_dir}/pedtex

install -d -m 755 $RPM_BUILD_ROOT%{reports_dir}/ps-fan
install -m 644 reports/ps-fan/ps-fan* $RPM_BUILD_ROOT%{reports_dir}/ps-fan

install -d -m 755 $RPM_BUILD_ROOT%{tt_dir}
install -m 644 tt/*.tt $RPM_BUILD_ROOT%{tt_dir}
install -d -m 755 $RPM_BUILD_ROOT/usr/local/man/man1
install -m 644 docs/btedit.1 $RPM_BUILD_ROOT/usr/local/man/man1/btedit.1
install -m 644 docs/dbverify.1 $RPM_BUILD_ROOT/usr/local/man/man1/dbverify.1
install -m 644 docs/llines.1 $RPM_BUILD_ROOT/usr/local/man/man1/llines.1
install -m 644 docs/llexec.1 $RPM_BUILD_ROOT/usr/local/man/man1/llexec.1


%files
%defattr(-,root,root)
%doc README ChangeLog NEWS AUTHORS LICENSE
%doc README.AUTOCONF README.DEVELOPERS README.INTERNATIONAL README.LAYOUT
%doc README.MAINTAINERS README.MAINTAINERS.rpm README.MAINTAINERS.win32
%doc .linesrc docs/lifelines.vim
%doc docs/ll-reportmanual.xml docs/ll-reportmanual.html docs/ll-reportmanual.pdf docs/ll-reportmanual.txt
%doc docs/ll-userguide.xml docs/ll-userguide.html docs/ll-userguide.pdf docs/ll-userguide.txt

/usr/local/bin/llines
/usr/local/bin/llexec
/usr/local/bin/dbverify
/usr/local/share/locale/da/LC_MESSAGES/lifelines.mo
/usr/local/share/locale/de/LC_MESSAGES/lifelines.mo
/usr/local/share/locale/fr/LC_MESSAGES/lifelines.mo
/usr/local/share/locale/sv/LC_MESSAGES/lifelines.mo
/usr/local/share/locale/eo/LC_MESSAGES/lifelines.mo
/usr/local/share/locale/es/LC_MESSAGES/lifelines.mo
/usr/local/share/locale/nl/LC_MESSAGES/lifelines.mo
/usr/local/share/locale/pl/LC_MESSAGES/lifelines.mo
/usr/local/share/locale/rw/LC_MESSAGES/lifelines.mo
%{reports_dir}
%{tt_dir}
/usr/local/man/man1/btedit.1
/usr/local/man/man1/dbverify.1
/usr/local/man/man1/llines.1
/usr/local/man/man1/llexec.1

%changelog
* Sun Apr 29 2007 Perry Rapp
- add llexec.1 man page to release
* Sun Apr 15 2007 Stephen Dum
- add README* files to release
* Wed Dec 21 2005 Stephen Dum
- add .tt files, .linesrc and new .mo files to release
* Tue Nov 15 2005 Perry Rapp
- Change "Copyright" to newer "License".
- Fix license name to clearer "X11".
- Add BuildRequires line.
* Thu Sep 29 2005 Perry Rapp
- Add eo, es, nl, pl, rw translations. Add *.li files. Add novel, pedtex, ps-fan reports.
* Sun Jan 30 2005 Perry Rapp
- Add comment about using rpmbuild on newer systems, and instruction about adding comments down here.
* Tue May 06 2003 Perry Rapp
- Add btedit.1 (& alphabetize man pages)
* Mon May 05 2003 Perry Rapp
- Add dbverify.1
* Mon Nov 11 2002 Perry Rapp
- Add da.po
- (post lifelines-3.0.21)
* Mon Oct 07 2002 Perry Rapp
- Add new binary llexec
* Sun Sep 29 2002 Perry Rapp
- Implement patch by Karl DeBisschop: adds build root, list reports as group instead of individually
- Add dbverify, de.po, fr.po, sv.po
- (lifelines-3.0.19-1)

