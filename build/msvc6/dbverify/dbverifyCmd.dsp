# Microsoft Developer Studio Project File - Name="dbverifyCmd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=dbverifyCmd - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dbverifyCmd.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dbverifyCmd.mak" CFG="dbverifyCmd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dbverifyCmd - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "dbverifyCmd - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dbverifyCmd - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\obj\dbverify\Release"
# PROP Intermediate_Dir "..\..\..\..\obj\dbverify\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /I "../../../src/hdrs" /I "../../../src/hdrs/win32" /I "../../../src/intl" /I "." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "HAVE_CONFIG_H" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib version.lib /nologo /subsystem:console /map /debug /machine:I386 /out:"..\..\..\..\bin\dbverify\Release\dbverify.exe" /libpath:"..\..\..\..\bin\libintl\Release" /libpath:"..\..\..\iconv.rel\lib" /mapinfo:lines
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "dbverifyCmd - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\obj\dbverify\Debug"
# PROP Intermediate_Dir "..\..\..\..\obj\dbverify\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../../../src/hdrs" /I "../../../src/hdrs/win32" /I "../../../src/intl" /I "." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "HAVE_CONFIG_H" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib version.lib /nologo /subsystem:console /debug /machine:I386 /out:"..\..\..\..\bin\dbverify\Debug\dbverify.exe" /pdbtype:sept /libpath:"..\..\..\..\bin\libintl\Debug" /libpath:"..\..\..\iconv.dbg\lib"

!ENDIF 

# Begin Target

# Name "dbverifyCmd - Win32 Release"
# Name "dbverifyCmd - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\btree\addkey.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\arch\alphasort.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\appendstr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\btree\block.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\brwslist.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\btree\btrec.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\charmaps.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\charprops.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\codesets.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\datei.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\dateparse.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\dateprint.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\dbcontext.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\tools\dbverify.c
# End Source File
# Begin Source File

SOURCE=.\dbVerify.rc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\dirs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\environ.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\equaliso.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\errlog.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\btree\file.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\fileops.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\fpattern.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\gedcom.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\generic.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\gengedc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\gstrings.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\hashtab.c
# End Source File
# Begin Source File

SOURCE=..\..\..\win32\iconvshim.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\icvt.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\btree\index.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\indiseq.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\init.c
# End Source File
# Begin Source File

SOURCE=..\..\..\win32\intlshim.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\intrface.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\keytonod.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\list.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\listener.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\llabort.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\lldatabase.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\lldate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\llgettext.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\lloptions.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\llstrcmp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\locales.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\log.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\memalloc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\messages.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\misc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\mychar_funcs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\mychar_tables.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\mystring.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\names.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\arch\nl_langinfo.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\node.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\nodechk.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\nodeio.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\nodeutls.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\object.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\btree\opnbtree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\path.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\place.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\arch\platform.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\property.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\rbtree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\record.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\refns.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\remove.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\replace.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\arch\scandir.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\sequence.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\soundex.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\spltjoin.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\sprintpic.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\stack.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\stdlib.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\stdstrng.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\stralloc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\strapp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\strcvt.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\strset.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\strutf8.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\strwhite.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\table.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\translat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\btree\traverse.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\btree\utils.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\valid.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\valtable.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\arch\vsnprintf.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\vtable.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\tools\wprintf.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\xlat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\xreffile.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\zstr.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\src\hdrs\arch.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\btree.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\btree\btreei.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\cache.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\charprops.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\codesets.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\date.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\datei.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\dbcontext.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\feedback.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\fpattern.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\gedcheck.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\gedcom.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\gedcom_macros.h
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\gedcomi.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\gedlib\gedcomi.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\generic.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\gengedc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\hashtab.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\win32\iconv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\win32\iconvshim.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\icvt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\indiseq.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\interp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\win32\intlshim.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\isolangs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\langinfz.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\liflines.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\llnls.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\lloptions.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\llstdlib.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\log.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\menuitem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\metadata.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\msvc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\mychar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\mystring.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\object.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\rbtree.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\sequence.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\stack.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\standard.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\stdlibi.h
# End Source File
# Begin Source File

SOURCE=D:\dev\PlatformSdk\Include\StrAlign.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\sys_inc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\table.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\translat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\uiprompts.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\vtable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\xlat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\zstr.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\llines.ico
# End Source File
# End Group
# End Target
# End Project
