# Microsoft Developer Studio Project File - Name="IceUtil" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=IceUtil - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "IceUtil.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "IceUtil.mak" CFG="IceUtil - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "IceUtil - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "IceUtil - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "IceUtil - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I ".." /I "../../include" /D "_USRDLL" /D "ICE_UTIL_API_EXPORTS" /D "NDEBUG" /D "_CONSOLE" /D "_UNICODE" /FD /c
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
# ADD LINK32 rpcrt4.lib /nologo /dll /machine:I386 /out:"Release/iceutil12.dll" /implib:"Release/iceutil.lib"
# SUBTRACT LINK32 /pdb:none /debug /nodefaultlib
# Begin Special Build Tool
OutDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\iceutil.lib ..\..\lib	copy $(OutDir)\iceutil12.dll ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "_USRDLL" /D "ICE_UTIL_API_EXPORTS" /D "_DEBUG" /D "_CONSOLE" /D "_UNICODE" /FD /GZ /c
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
# ADD LINK32 rpcrt4.lib /nologo /dll /debug /machine:I386 /out:"Debug/iceutil12d.dll" /implib:"Debug/iceutild.lib"
# SUBTRACT LINK32 /pdb:none /nodefaultlib
# Begin Special Build Tool
OutDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\iceutild.lib ..\..\lib	copy $(OutDir)\iceutil12d.pdb ..\..\bin	copy $(OutDir)\iceutil12d.dll ..\..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "IceUtil - Win32 Release"
# Name "IceUtil - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Base64.cpp
# End Source File
# Begin Source File

SOURCE=.\Cond.cpp
# End Source File
# Begin Source File

SOURCE=.\CtrlCHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\Exception.cpp
# End Source File
# Begin Source File

SOURCE=.\GC.cpp
# End Source File
# Begin Source File

SOURCE=.\InputUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectBase.cpp
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

SOURCE=.\StaticMutex.cpp
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

SOURCE=..\..\include\IceUtil\Base64.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Cond.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Config.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\CtrlCHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Exception.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Functional.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\GC.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\GCF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Handle.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\IceUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\InputUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Lock.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Monitor.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Mutex.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\ObjectBase.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\OutputUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\RecMutex.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\RWRecMutex.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Shared.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\StaticMutex.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Thread.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\ThreadException.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Time.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Unicode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\UUID.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
