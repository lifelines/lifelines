' create_lifelines_release.vbs
' Create new lifelines release directory
' Created: 2005-06-12, Perry
' Edited:  2005-09-16, Perry
'
' This requires a working cygwin directory (for tarball and gmo language files)
' and a Windows source directory (for the bin output directory of Visual C++)
' both paths must be passed in to this script

Option Explicit

Call Main

Sub Main

  Dim fso
  Set fso = CreateObject("Scripting.FileSystemObject")

  ' We require a cygwin installation to have compiled a release tarball

  Dim CygwinLifelinesRoot
  CygwinLifelinesRoot = WScript.Arguments.Named("CygwinLifelinesRoot")
  If CygwinLifelinesRoot = vbEmpty Then
    Call ShowUsage
    Exit Sub
  End If

  If Not fso.FolderExists(CygwinLifelinesRoot) Then
    Fail "Specified cygwin root folder does not exist: " + CygwinLifelinesRoot
    Exit Sub
  End If

  ' We require a source directory used by Visual C++ compiler
  ' so we can read files from its ..\bin folder

  Dim Win32LifelinesRoot
  Win32LifelinesRoot = WScript.Arguments.Named("Win32LifelinesRoot")
  If Win32LifelinesRoot = vbEmpty Then
    Call ShowUsage
    Exit Sub
  End If

  If Not CheckRequiredFolder(fso, Win32LifelinesRoot, "Win32 lifelines cvs source") Then
    Exit Sub
  End If

  Dim Win32binariesFolder
  Win32binariesFolder = fso.GetParentFolderName(Win32LifelinesRoot) + "\bin"
  If Not CheckRequiredFolder(fso, Win32binariesFolder, "Win32 lifelines binaries folder root") Then
    Exit Sub
  End If
  If Not CheckRequiredFile(fso, Win32binariesFolder + "\dbverify\release\dbverify.exe", "Win32 dbverify binary") Then
    Exit Sub
  End If
  If Not CheckRequiredFolder(fso, Win32binariesFolder + "\llines\release", "Win32 llines binaries folder") Then
    Exit Sub
  End If
  If Not CheckRequiredFile(fso, Win32binariesFolder + "\llines\release\iconv.dll", "Win32 iconv binary") Then
    Exit Sub
  End If
  If Not CheckRequiredFile(fso, Win32binariesFolder + "\llines\release\libintl.dll", "Win32 gettext binary") Then
    Exit Sub
  End If
  If Not CheckRequiredFile(fso, Win32binariesFolder + "\llines\release\localcharset.dll", "Win32 localcharset binary") Then
    Exit Sub
  End If

  Dim release
  release = InputBox("New release version")

  If Len(release)<3 Then
    Output "Aborting"
    Exit Sub
  End If

  Dim langs
  langs = PopulateLangs

  ' Check that gmo (compiled message catalogs) exist
  Dim errstr
  errstr = CheckLangs(CygwinLifelinesRoot, fso, langs)
  If Len(errstr) > 0 Then
    Fail errstr
    Exit Sub
  End If

  Dim releaseName
  releaseName = "lifelines-" & release

  Dim tarball
  tarball = releaseName + ".tar.gz"

  ' Check that release tarball exists
  Dim cygTarball
  cygTarball = CygwinLifelinesRoot + "\bld\" + tarball
  If Not fso.FileExists(cygTarball) Then
    Fail "Missing distribution tarball: " + cygTarball
    Exit Sub
  End If

  Dim root
  root = release

  If fso.FolderExists(root) Then
    Dim msg, rtn
	msg = "Directory " & release & " already exists. Delete & recreate?"
    rtn = MsgBox(msg, MB_YESNO, "Release Already Exists")
	If rtn = IDYES Then
	  fso.DeleteFolder(root)
	Else
	  Exit Sub
	End If
  End If

  ' Create directories
  fso.CreateFolder root
  fso.CopyFile cygTarball, root + "\"
  fso.CreateFolder root + "\" + releaseName

  Dim root1, root2
  root1 = root + "\" + releaseName & "-1.bin.win32"
  root2 = root + "\" + releaseName & "-1.bin.win32.exe_only"

  CreateReleaseFolders fso, root1
  CreateReleaseFolders fso, root2

  root1 = root1 + "\lifelines"
  root2 = root2 + "\lifelines"

  CopyExes fso, Win32binariesFolder, root1
  CopyExes fso, Win32binariesFolder, root2

  CopyLangs fso, CygwinLifelinesRoot, langs, root1
  CopyLangs fso, CygwinLifelinesRoot, langs, root2

  ' TODO:
  '  Docs
  '  reports
  '  config & readmes in root

End Sub

' Create all folders down through direct children of lifelines folder
Sub CreateReleaseFolders(fso, rootFolder)
  fso.CreateFolder rootFolder
  fso.CreateFolder rootFolder + "\lifelines"
  fso.CreateFolder rootFolder + "\lifelines\Archives"
  fso.CreateFolder rootFolder + "\lifelines\Databases"
  fso.CreateFolder rootFolder + "\lifelines\Docs"
  fso.CreateFolder rootFolder + "\lifelines\locale"
  fso.CreateFolder rootFolder + "\lifelines\Outputs"
  fso.CreateFolder rootFolder + "\lifelines\Programs"
  fso.CreateFolder rootFolder + "\lifelines\tt"
End Sub


' If specified folder does not exist, do msgbox & return false
Function CheckRequiredFolder(fso, folder, folderDesc)
  If fso.FolderExists(folder) Then
    CheckRequiredFolder = True
  Else
    Fail "Required " + folderDesc + " does not exist: " + folder
    CheckRequiredFolder = False
  End If
End Function

' If specified folder does not exist, do msgbox & return false
Function CheckRequiredFile(fso, file, fileDesc)
  If fso.FileExists(file) Then
    CheckRequiredFile = True
  Else
    Fail "Required " + fileDesc + " does not exist: " + file
    CheckRequiredFile = False
  End If
End Function

' Dimension & populate array of language codes
' (for languages in which lifelines had translation)
Function PopulateLangs()
  Dim langs()
  Dim langString
  langString = "da de el fr it nl pl rw sv"
  Dim nlangs
  nlangs = (Len(langString)+1)/3
  ReDim langs(nlangs-1)
  Dim i
  For i=0 To UBound(langs, 1)
    langs(i) = Mid(langString, 1 + i*3, 2)
  Next
  PopulateLangs = langs
End Function

' Returns a string with error message, or empty string if ok
Function CheckLangs(CygwinLifelinesRoot, fso, langs)
  Dim i
  Dim str
  str = ""
  For i=0 To UBound(langs,1)
    Dim gmofile
    gmofile = CygwinLifelinesRoot + "\po\" + langs(i) + ".gmo"
    If Not fso.FileExists(gmofile) Then
      If Len(str) > 0 Then
        str = str + VbCrLf
      End If
      str = str + "Missing gmo file: " + gmofile
    End If
  Next
  CheckLangs = str
End Function

' Copy translated message catalogs
Sub CopyLangs(fso, CygwinLifelinesRoot, langs, outputFolder)
  Dim i
  Dim target
  target = outputFolder + "\locale"
  fso.CreateFolder target
  For i=0 To UBound(langs,1)
    Dim gmofile
    gmofile = CygwinLifelinesRoot + "\po\" + langs(i) + ".gmo"
    Dim targ2
    targ2 = target + "\" + langs(i)
    fso.CreateFolder targ2
    targ2 = targ2 + "\LC_MESSAGES"
    fso.CreateFolder targ2
    targ2 = targ2 + "\lifelines.mo"
    fso.CopyFile gmofile, targ2
  Next
End Sub

' Copy actual win32 binary files
Sub CopyExes(fso, Win32binariesFolder, outputFolder)
  Dim target
  target = outputFolder + "\"
  fso.CopyFile Win32binariesFolder + "\dbverify\release\dbverify.exe", target
  fso.CopyFile Win32binariesFolder + "\llines\release\Lines.exe", target
  fso.CopyFile Win32binariesFolder + "\dbverify\release\llexec.exe", target
End Sub

' Bare wrapper for creating a directory
Sub CreateFolder(fso, folder)
  fso.CreateFolder folder
End Sub

Sub Output(msg)
  WScript.Echo msg
End Sub

Sub Fail(msg)
  MsgBox msg, MB_ICONSTOP
End Sub

' Display via Echo information showing how to invoke this file
Sub ShowUsage
  Dim str
  str = "create_lifelines_release.vbs /CygwinLifelinesRoot:somepath"
  WScript.Echo str
End Sub


' MsgBox parameters
Const MB_OK = 0
Const MB_OKCANCEL = 1
Const MB_ABORTRETRYIGNORE = 2
Const MB_YESNOCANCEL = 3
Const MB_YESNO = 4
Const MB_RETRYCANCEL = 5

Const MB_ICONSTOP = 16
Const MB_ICONQUESTION = 32
Const MB_ICONEXCLAMATION = 48
Const MB_ICONINFORMATION = 64

Const MB_DEFBUTTON1 = 0
Const MB_DEFBUTTON2 = 256
Const MB_DEFBUTTON3 = 512

' MsgBox return values
Const IDOK = 1
Const IDCANCEL = 2
Const IDABORT = 3
Const IDRETRY = 4
Const IDIGNORE = 5
Const IDYES = 6
Const IDNO = 7