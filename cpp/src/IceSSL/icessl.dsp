# Microsoft Developer Studio Project File - Name="icessl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=icessl - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "icessl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "icessl.mak" CFG="icessl - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "icessl - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "icessl - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "icessl - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I "." /I ".." /I "../../include" /D "USE_SOCKETS" /D "_USRDLL" /D "ICE_SSL_API_EXPORTS" /D FD_SETSIZE=1024 /D "_CONSOLE" /D "NDEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /c
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
# ADD LINK32 ws2_32.lib libeay32.lib ssleay32.lib /nologo /dll /pdb:none /machine:I386 /out:"Release/icessl31.dll" /implib:"Release/icessl.lib" /FIXED:no
# SUBTRACT LINK32 /debug
# Begin Special Build Tool
OutDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\icessl.lib ..\..\lib	copy $(OutDir)\icessl31.dll ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "icessl - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I "." /I ".." /I "../../include" /D "USE_SOCKETS" /D "_USRDLL" /D "ICE_SSL_API_EXPORTS" /D FD_SETSIZE=1024 /D "_CONSOLE" /D "_DEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /GZ /c
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
# ADD LINK32 ws2_32.lib libeay32.lib ssleay32.lib /nologo /dll /debug /machine:I386 /out:"Debug/icessl31d.dll" /implib:"Debug/icessld.lib" /FIXED:no
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
OutDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\icessld.lib ..\..\lib	copy $(OutDir)\icessl31d.pdb ..\..\bin	copy $(OutDir)\icessl31d.dll ..\..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "icessl - Win32 Release"
# Name "icessl - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AcceptorI.cpp
# End Source File
# Begin Source File

SOURCE=.\Certificate.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnectorI.cpp
# End Source File
# Begin Source File

SOURCE=.\EndpointI.cpp
# End Source File
# Begin Source File

SOURCE=.\Instance.cpp
# End Source File
# Begin Source File

SOURCE=.\PluginI.cpp
# End Source File
# Begin Source File

SOURCE=.\TransceiverI.cpp
# End Source File
# Begin Source File

SOURCE=.\Util.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AcceptorI.h
# End Source File
# Begin Source File

SOURCE=.\ConnectorI.h
# End Source File
# Begin Source File

SOURCE=.\EndpointI.h
# End Source File
# Begin Source File

SOURCE=.\Instance.h
# End Source File
# Begin Source File

SOURCE=.\InstanceF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceSSL\Plugin.h
# End Source File
# Begin Source File

SOURCE=.\PluginI.h
# End Source File
# Begin Source File

SOURCE=.\TransceiverI.h
# End Source File
# Begin Source File

SOURCE=.\Util.h
# End Source File
# Begin Source File

SOURCE=.\UtilF.h
# End Source File
# End Group
# End Target
# End Project
