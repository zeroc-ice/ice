# Microsoft Developer Studio Project File - Name="Glacier" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Glacier - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Glacier.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Glacier.mak" CFG="Glacier - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Glacier - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Glacier - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Glacier - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I ".." /I "../../include" /D "NDEBUG" /D "_USRDLL" /D "GLACIER_API_EXPORTS" /D "_CONSOLE" /D "_UNICODE" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 /nologo /dll /machine:I386 /out:"Release/glacier001.dll" /libpath:"../../../lib"
# SUBTRACT LINK32 /pdb:none /debug /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\glacier001.* ..\..\lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Glacier - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "_DEBUG" /D "_USRDLL" /D "GLACIER_API_EXPORTS" /D "_CONSOLE" /D "_UNICODE" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /dll /debug /machine:I386 /out:"Debug/glacier001d.dll" /pdbtype:sept /libpath:"../../../lib"
# SUBTRACT LINK32 /pdb:none /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\glacier001d.* ..\..\lib
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Glacier - Win32 Release"
# Name "Glacier - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Router.cpp
# End Source File
# Begin Source File

SOURCE=.\Session.cpp
# End Source File
# Begin Source File

SOURCE=.\SessionManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Starter.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\Glacier\Glacier.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Glacier\Router.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Glacier\Session.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Glacier\SessionF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Glacier\SessionManager.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Glacier\SessionManagerF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Glacier\Starter.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\slice\Glacier\Router.ice

!IF  "$(CFG)" == "Glacier - Win32 Release"

USERDEP__ROUTE="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Glacier\Router.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/Router.ice \
	move Router.h ..\..\include\Glacier \
	

"..\..\include\Glacier\Router.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Router.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Glacier - Win32 Debug"

USERDEP__ROUTE="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Glacier\Router.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/Router.ice \
	move Router.h ..\..\include\Glacier \
	

"..\..\include\Glacier\Router.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Router.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Glacier\Session.ice

!IF  "$(CFG)" == "Glacier - Win32 Release"

USERDEP__SESSI="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Glacier\Session.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/Session.ice \
	move Session.h ..\..\include\Glacier \
	

"..\..\include\Glacier\Session.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Session.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Glacier - Win32 Debug"

USERDEP__SESSI="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Glacier\Session.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/Session.ice \
	move Session.h ..\..\include\Glacier \
	

"..\..\include\Glacier\Session.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Session.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Glacier\SessionF.ice

!IF  "$(CFG)" == "Glacier - Win32 Release"

USERDEP__SESSIO="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Glacier\SessionF.ice

"..\..\include\Glacier\SessionF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/SessionF.ice 
	move SessionF.h ..\..\include\Glacier 
	del SessionF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Glacier - Win32 Debug"

USERDEP__SESSIO="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Glacier\SessionF.ice

"..\..\include\Glacier\SessionF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/SessionF.ice 
	move SessionF.h ..\..\include\Glacier 
	del SessionF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Glacier\SessionManager.ice

!IF  "$(CFG)" == "Glacier - Win32 Release"

USERDEP__SESSION="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Glacier\SessionManager.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/SessionManager.ice \
	move SessionManager.h ..\..\include\Glacier \
	

"..\..\include\Glacier\SessionManager.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"SessionManager.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Glacier - Win32 Debug"

USERDEP__SESSION="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Glacier\SessionManager.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/SessionManager.ice \
	move SessionManager.h ..\..\include\Glacier \
	

"..\..\include\Glacier\SessionManager.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"SessionManager.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Glacier\SessionManagerF.ice

!IF  "$(CFG)" == "Glacier - Win32 Release"

USERDEP__SESSIONM="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Glacier\SessionManagerF.ice

"..\..\include\Glacier\SessionManagerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/SessionManagerF.ice 
	move SessionManagerF.h ..\..\include\Glacier 
	del SessionManagerF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Glacier - Win32 Debug"

USERDEP__SESSIONM="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Glacier\SessionManagerF.ice

"..\..\include\Glacier\SessionManagerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/SessionManagerF.ice 
	move SessionManagerF.h ..\..\include\Glacier 
	del SessionManagerF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Glacier\Starter.ice

!IF  "$(CFG)" == "Glacier - Win32 Release"

USERDEP__START="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Glacier\Starter.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/Starter.ice \
	move Starter.h ..\..\include\Glacier \
	

"..\..\include\Glacier\Starter.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Starter.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Glacier - Win32 Debug"

USERDEP__START="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Glacier\Starter.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/Starter.ice \
	move Starter.h ..\..\include\Glacier \
	

"..\..\include\Glacier\Starter.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Starter.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
