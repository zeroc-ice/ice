# Microsoft Developer Studio Project File - Name="iceutil" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=iceutil - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iceutil.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iceutil.mak" CFG="iceutil - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iceutil - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "iceutil - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iceutil - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBRARY_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /WX /GR /GX /Zi /O2 /I ".." /I "../../include" /D "_USRDLL" /D "ICE_UTIL_API_EXPORTS" /D "_CONSOLE" /D "NDEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 rpcrt4.lib /nologo /dll /incremental:yes /debug /machine:I386 /out:"Release/iceutil30.dll" /implib:"Release/iceutil.lib" /FIXED:no
# SUBTRACT LINK32 /pdb:none  /nodefaultlib
# Begin Special Build Tool
OutDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\iceutil.lib ..\..\lib	copy $(OutDir)\iceutil30.dll ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "iceutil - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBRARY_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "_USRDLL" /D "ICE_UTIL_API_EXPORTS" /D "_CONSOLE" /D "_DEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /GZ /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386
# ADD LINK32 rpcrt4.lib /nologo /dll /debug /machine:I386 /out:"Debug/iceutil30d.dll" /implib:"Debug/iceutild.lib" /FIXED:no
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
OutDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\iceutild.lib ..\..\lib	copy $(OutDir)\iceutil30d.pdb ..\..\bin	copy $(OutDir)\iceutil30d.dll ..\..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "iceutil - Win32 Release"
# Name "iceutil - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Base64.cpp
# End Source File
# Begin Source File

SOURCE=.\Cond.cpp
# End Source File
# Begin Source File

SOURCE=.\CountDownLatch.cpp
# End Source File
# Begin Source File

SOURCE=.\CtrlCHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\Exception.cpp
# End Source File
# Begin Source File

SOURCE=.\InputUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\MD5.cpp
# End Source File
# Begin Source File

SOURCE=.\MD5I.cpp
# End Source File
# Begin Source File

SOURCE=.\Options.cpp
# End Source File
# Begin Source File

SOURCE=.\OutputUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\RecMutex.cpp
# End Source File
# Begin Source File

SOURCE=.\RWRecMutex.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared.cpp
# End Source File
# Begin Source File

SOURCE=.\StaticMutex.cpp
# End Source File
# Begin Source File

SOURCE=.\StringUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\Thread.cpp
# End Source File
# Begin Source File

SOURCE=.\ThreadException.cpp
# End Source File
# Begin Source File

SOURCE=.\Time.cpp
# End Source File
# Begin Source File

SOURCE=.\Unicode.cpp
# End Source File
# Begin Source File

SOURCE=.\UUID.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\iceutil\Base64.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\Cond.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\Config.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\CountDownLatch.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\CtrlCHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\Exception.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\Functional.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\Handle.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\IceUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\InputUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\Lock.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\MD5.h
# End Source File
# Begin Source File

SOURCE=MD5I.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\Monitor.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\Mutex.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Options.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\OutputUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\RecMutex.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\RWRecMutex.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\Shared.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\StaticMutex.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\StringUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\Thread.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\ThreadException.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\Time.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\Unicode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\UUID.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
