# Microsoft Developer Studio Project File - Name="omniORBS" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=omniORBS - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "omniORBS.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "omniORBS.mak" CFG="omniORBS - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "omniORBS - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "omniORBS - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "omniORBS - Win32 Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "."
# PROP Intermediate_Dir "Debug\omniORBS"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD MTL /D "_DEBUG" /nologo /mktyplib203 /win32
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Gy /I "." /I "$(OMNIORB_HOME)\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "__x86__" /D "__OMNIORB4__" /D "__WIN32__" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /i "$(TAO_ROOT)\.." /i "$(TAO_ROOT)" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 advapi32.lib user32.lib omniORB407_rtd.lib omnithread32_rtd.lib msvcstubd.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /out:".\server.exe" /libpath:"$(OMNIORB_HOME)\lib\x86_win32"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "omniORBS - Win32 Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD MTL /D "NDEBUG" /nologo /mktyplib203 /win32
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "." /I "$(OMNIORB_HOME)\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "__x86__" /D "__OMNIORB4__" /D "__WIN32__" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /i "$(TAO_ROOT)\.." /i "$(TAO_ROOT)" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 advapi32.lib user32.lib omniORB407_rt.lib omnithread32_rt.lib msvcstub.lib /nologo /subsystem:console /pdb:none /machine:I386 /out:"server.exe" /libpath:"$(OMNIORB_HOME)\lib\x86_win32"

!ENDIF 

# Begin Target

# Name "omniORBS - Win32 Debug"
# Name "omniORBS - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;cxx;c"
# Begin Source File

SOURCE=.\pingSK.cpp
# End Source File
# Begin Source File

SOURCE=.\Server.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hh"
# Begin Source File

SOURCE=.\ping.hh
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ping.idl

!IF  "$(CFG)" == "omniORBS - Win32 Debug"

# Begin Custom Build
InputPath=.\ping.idl

BuildCmds= \
	$(OMNIORB_HOME)\bin\x86_win32\omniidl  -bcxx -Wba ping.idl \
	move pingSK.cc pingSK.cpp \
	

"pingSK.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pingSK.hh" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "omniORBS - Win32 Release"

# Begin Custom Build
InputPath=.\ping.idl

BuildCmds= \
	$(OMNIORB_HOME)\bin\x86_win32\omniidl  -bcxx -Wba ping.idl \
	move pingSK.cc pingSK.cpp \
	

"pingSK.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pingSK.hh" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
