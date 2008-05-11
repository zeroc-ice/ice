# Microsoft Developer Studio Project File - Name="destroyer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=destroyer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "destroyer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "destroyer.mak" CFG="destroyer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "destroyer - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "destroyer - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "destroyer - Win32 Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "."
# PROP Intermediate_Dir "Debug\destroyer"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD MTL /D "_DEBUG" /nologo /mktyplib203 /win32
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Gy /I "$(TAO_ROOT)\.." /I "$(TAO_ROOT)" /I "$(TAO_ROOT)\orbsvcs" /I "$(ICE_HOME)\include" /I "." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /i "$(TAO_ROOT)\.." /i "$(TAO_ROOT)" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 TAO_CosEventd.lib TAOd.lib TAO_Messagingd.lib ACEd.lib advapi32.lib user32.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /out:".\destroyer.exe" /libpath:"$(TAO_ROOT)\..\lib" /libpath:"$(TAO_ROOT)\..\ace" /libpath:"$(TAO_ROOT)\tao" /libpath:"$(TAO_ROOT)\tao\Strategies" /libpath:"$(TAO_ROOT)\tao\Messaging" /libpath:"$(TAO_ROOT)\tao\ValueType" /libpath:"$(TAO_ROOT)\tao\PortableServer" /libpath:"$(ICE_HOME)\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "destroyer - Win32 Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD MTL /D "NDEBUG" /nologo /mktyplib203 /win32
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(TAO_ROOT)\orbsvcs" /I "$(TAO_ROOT)\.." /I "$(TAO_ROOT)" /I "$(ICE_HOME)\include" /I "." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /i "$(TAO_ROOT)\.." /i "$(TAO_ROOT)" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 TAO_CosEvent.lib TAO_AnyTypeCode.lib TAO_Messaging.lib TAO.lib ACE.lib advapi32.lib user32.lib /nologo /subsystem:console /pdb:none /machine:I386 /out:"destroyer.exe" /libpath:"$(TAO_ROOT)\..\lib" /libpath:"$(TAO_ROOT)\..\ace" /libpath:"$(TAO_ROOT)\tao" /libpath:"$(TAO_ROOT)\tao\Strategies" /libpath:"$(TAO_ROOT)\tao\Messaging" /libpath:"$(TAO_ROOT)\tao\ValueType" /libpath:"$(TAO_ROOT)\tao\PortableServer" /libpath:"$(ICE_HOME)\lib"

!ENDIF 

# Begin Target

# Name "destroyer - Win32 Debug"
# Name "destroyer - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;cxx;c"
# Begin Source File

SOURCE=.\Destroyer.cpp
# End Source File
# End Group
# End Target
# End Project
