# Microsoft Developer Studio Project File - Name="llinesprj" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=llinesprj - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "llinesprj.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "llinesprj.mak" CFG="llinesprj - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "llinesprj - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "llinesprj - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "llinesprj - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\obj\llines\Release"
# PROP Intermediate_Dir "..\..\..\..\obj\llines\Release"
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib libintlvc6.lib iconv.lib /nologo /subsystem:console /map /debug /machine:I386 /out:"..\..\..\..\bin\llines\Release\Lines.exe" /libpath:"..\..\..\..\bin\libintl\Release" /libpath:"..\..\..\iconv.rel\lib" /mapinfo:lines
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "llinesprj - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\obj\llines\Debug"
# PROP Intermediate_Dir "..\..\..\..\obj\llines\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../../../hdrs" /I "../../../hdrs/win32" /I "../../../intl" /I "." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "HAVE_CONFIG_H" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib libintlvc6.lib iconv.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"msvcrt.lib" /nodefaultlib:"libcmtd.lib" /nodefaultlib:"msvcrtd.lib" /out:"..\..\..\..\bin\llines\Debug/llines.exe" /pdbtype:sept /libpath:"..\..\..\..\bin\libintl\Debug" /libpath:"..\..\..\iconv.dbg\lib"

!ENDIF 

# Begin Target

# Name "llinesprj - Win32 Release"
# Name "llinesprj - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\liflines\add.c
# End Source File
# Begin Source File

SOURCE=..\..\..\btree\addkey.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\advedit.c
# End Source File
# Begin Source File

SOURCE=..\..\..\interp\alloc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\arch\alphasort.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\ask.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\askprogram.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\bfs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\btree\block.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\browse.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\brwslist.c
# End Source File
# Begin Source File

SOURCE=..\..\..\interp\builtin.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\charmaps.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\choose.c
# End Source File
# Begin Source File

SOURCE=..\..\..\interp\date.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\delete.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\dirs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\double.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\edit.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\editmap.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\editvtab.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\environ.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\equaliso.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\error.c
# End Source File
# Begin Source File

SOURCE=..\..\..\interp\eval.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\export.c
# End Source File
# Begin Source File

SOURCE=..\..\..\btree\file.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\fpattern.c
# End Source File
# Begin Source File

SOURCE=..\..\..\interp\functab.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\gedcom.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\gengedc.c
# End Source File
# Begin Source File

SOURCE=..\..\getopt.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\gstrings.c
# End Source File
# Begin Source File

SOURCE=..\..\..\interp\heapused.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\import.c
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

SOURCE=..\..\..\interp\interp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\intrface.c
# End Source File
# Begin Source File

SOURCE=..\..\..\interp\intrpseq.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\keytonod.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\lbrowse.c
# End Source File
# Begin Source File

SOURCE=..\..\..\interp\lex.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\lldate.c
# End Source File
# Begin Source File

SOURCE=.\llines.rc
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\lloptions.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\llstrcmp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\loadsave.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\main.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\memalloc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\menuitem.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\merge.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\messages.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\misc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\miscutls.c
# End Source File
# Begin Source File

SOURCE=..\..\..\interp\more.c
# End Source File
# Begin Source File

SOURCE=..\..\mycurses.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\mystring.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\names.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\newrecs.c
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

SOURCE=..\..\..\liflines\pedigree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\place.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\property.c
# End Source File
# Begin Source File

SOURCE=..\..\..\interp\pvalmath.c
# End Source File
# Begin Source File

SOURCE=..\..\..\interp\pvalue.c
# End Source File
# Begin Source File

SOURCE=..\..\..\interp\rassa.c
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

SOURCE=..\..\..\liflines\scan.c
# End Source File
# Begin Source File

SOURCE=..\..\..\arch\scandir.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\screen.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\sequence.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\show.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\signals.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\spltjoin.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\stdstrng.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\swap.c
# End Source File
# Begin Source File

SOURCE=..\..\..\interp\symtab.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\table.c
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\tandem.c
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

SOURCE=..\..\..\liflines\valgdcom.c
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

SOURCE=..\..\w32systm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\warehouse.c
# End Source File
# Begin Source File

SOURCE=..\..\..\interp\write.c
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\xreffile.c
# End Source File
# Begin Source File

SOURCE=..\..\..\interp\yacc.c
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\llines.ico
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\hdrs\arch.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\bfs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\btree.h
# End Source File
# Begin Source File

SOURCE=..\..\..\btree\btreei.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\cache.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\win32\curses.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\date.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\feedback.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\fpattern.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\gedcheck.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\gedcom.h
# End Source File
# Begin Source File

SOURCE=..\..\..\gedlib\gedcomi.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\gengedc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\win32\getopt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\indiseq.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\interp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\interp\interpi.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\liflines.h
# End Source File
# Begin Source File

SOURCE=..\..\..\liflines\llinesi.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\lloptions.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\llstdlib.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\menuitem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\metadata.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\msvc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\mystring.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\screen.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\sequence.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\standard.h
# End Source File
# Begin Source File

SOURCE=..\..\..\stdlib\stdlibi.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\sys_inc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\table.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\translat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\version.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hdrs\warehouse.h
# End Source File
# Begin Source File

SOURCE=..\..\..\interp\yacc.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\hdrs\impfeed.h
# End Source File
# End Target
# End Project
