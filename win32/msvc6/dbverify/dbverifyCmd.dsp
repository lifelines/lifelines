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
# ADD CPP /nologo /W3 /GX /Zi /O2 /I "../../../hdrs" /I "../../../hdrs/win32" /I "../../../intl" /I "." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "HAVE_CONFIG_H" /YX /FD /c
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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../../../hdrs" /I "../../../hdrs/win32" /I "../../../intl" /I "." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "HAVE_CONFIG_H" /YX /FD /GZ /c
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

SOURCE=..\..\..\btree\addkey.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\bfs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\btree\block.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\brwslist.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\charmaps.c
# End Source File
# Begin Source File

SOURCE=..\..\..\tools\dbverify.c
# End Source File
# Begin Source File

SOURCE=.\dbVerify.rc
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\dirs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\double.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\environ.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\equaliso.c
# End Source File
# Begin Source File

SOURCE=..\..\..\btree\file.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\fpattern.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\gedcom.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\gengedc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\gstrings.c
# End Source File
# Begin Source File

SOURCE=..\..\iconvshim.c
# End Source File
# Begin Source File

SOURCE=..\..\..\btree\index.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\indiseq.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\init.c
# End Source File
# Begin Source File

SOURCE=..\..\intlshim.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\intrface.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\keytonod.c
# End Source File
# Begin Source File

SOURCE=..\..\..\arch\langinfo.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\lldate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\lloptions.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\llstrcmp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\memalloc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\messages.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\misc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\mystring.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\names.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\node.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\nodeutls.c
# End Source File
# Begin Source File

SOURCE=..\..\..\btree\opnbtree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\path.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\place.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\property.c
# End Source File
# Begin Source File

SOURCE=..\..\..\btree\record.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\refns.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\remove.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\replace.c
# End Source File
# Begin Source File

SOURCE=..\..\..\arch\scandir.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\sequence.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\spltjoin.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\stdstrng.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\table.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\translat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\btree\traverse.c
# End Source File
# Begin Source File

SOURCE=..\..\..\btree\utils.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\valid.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\valtable.c
# End Source File
# Begin Source File

SOURCE=..\..\..\arch\vsnprintf.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\warehouse.c
# End Source File
# Begin Source File

SOURCE=..\..\..\tools\wprintf.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\xreffile.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\btree\btreei.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\gedcomi.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\stdlibi.h
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
