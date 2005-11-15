# Microsoft Developer Studio Project File - Name="bteditCmd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=bteditCmd - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bteditCmd.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bteditCmd.mak" CFG="bteditCmd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bteditCmd - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "bteditCmd - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bteditCmd - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\obj\btedit\Release"
# PROP Intermediate_Dir "..\..\..\..\obj\btedit\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\..\src\hdrs" /I "..\..\..\src\hdrs\win32" /I "." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "HAVE__VSNPRINTF" /D "HAVE_CONFIG_H" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib version.lib /nologo /subsystem:console /machine:I386 /out:"..\..\..\..\bin\btedit\Release\btedit.exe"

!ELSEIF  "$(CFG)" == "bteditCmd - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\obj\bteditCmd\Debug"
# PROP Intermediate_Dir "..\..\obj\bteditCmd\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\..\src\hdrs" /I "..\..\..\src\hdrs\win32" /I "." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "HAVE__VSNPRINTF" /D "HAVE_CONFIG_H" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib version.lib /nologo /subsystem:console /debug /machine:I386 /out:"..\..\bin\bteditCmd\Debug/bteditCmd.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "bteditCmd - Win32 Release"
# Name "bteditCmd - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\btree\addkey.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\btree\block.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\tools\btedit.c
# End Source File
# Begin Source File

SOURCE=.\btedit.rc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\btree\btrec.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\dirs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\environ.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\btree\file.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\fileops.c
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

SOURCE=..\..\..\win32\intlshim.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\llstrcmp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\memalloc.c
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

SOURCE=..\..\..\src\btree\opnbtree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\path.c
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

SOURCE=..\..\..\src\stdlib\strutf8.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\btree\utils.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\arch\vsnprintf.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\vtable.c
# End Source File
# Begin Source File

SOURCE=..\..\..\win32\w32systm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\tools\wprintf.c
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

SOURCE=..\..\..\src\hdrs\feedback.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\gedcom.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\interp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\llnls.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\llstdlib.h
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

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\hdrs\standard.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\stdlib\stdlibi.h
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
