# Microsoft Developer Studio Project File - Name="glacier" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=glacier - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "glacier.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "glacier.mak" CFG="Glacier - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "glacier - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "glacier - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "glacier - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I ".." /I "../../include" /D "NDEBUG" /D "_USRDLL" /D "GLACIER_API_EXPORTS" /D "_CONSOLE" /FD /c
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
# ADD LINK32 /nologo /dll /machine:I386 /out:"Release/glacier20.dll" /implib:"Release/glacier.lib"
# SUBTRACT LINK32 /pdb:none /debug /nodefaultlib
# Begin Special Build Tool
OutDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\glacier.lib ..\..\lib	copy $(OutDir)\glacier20.dll ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "glacier - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "_DEBUG" /D "_USRDLL" /D "GLACIER_API_EXPORTS" /D "_CONSOLE" /FD /GZ /c
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
# ADD LINK32 /nologo /dll /debug /machine:I386 /out:"Debug/glacier20d.dll" /implib:"Debug/glacierd.lib"
# SUBTRACT LINK32 /pdb:none /nodefaultlib
# Begin Special Build Tool
OutDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\glacierd.lib ..\..\lib	copy $(OutDir)\glacier20d.pdb ..\..\bin	copy $(OutDir)\glacier20d.dll ..\..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "glacier - Win32 Release"
# Name "glacier - Win32 Debug"
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

SOURCE=..\..\include\glacier\Router.h
# End Source File
# Begin Source File

SOURCE=..\..\include\glacier\Session.h
# End Source File
# Begin Source File

SOURCE=..\..\include\glacier\SessionF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\glacier\SessionManager.h
# End Source File
# Begin Source File

SOURCE=..\..\include\glacier\SessionManagerF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\glacier\Starter.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\slice\glacier\Router.ice

!IF  "$(CFG)" == "glacier - Win32 Release"

USERDEP__ROUTE="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier\Router.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --checksum --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/Router.ice \
	move Router.h ..\..\include\glacier \
	

"..\..\include\glacier\Router.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Router.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "glacier - Win32 Debug"

USERDEP__ROUTE="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier\Router.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --checksum --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/Router.ice \
	move Router.h ..\..\include\glacier \
	

"..\..\include\glacier\Router.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Router.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\glacier\Session.ice

!IF  "$(CFG)" == "glacier - Win32 Release"

USERDEP__SESSI="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier\Session.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --checksum --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/Session.ice \
	move Session.h ..\..\include\glacier \
	

"..\..\include\glacier\Session.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Session.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "glacier - Win32 Debug"

USERDEP__SESSI="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier\Session.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --checksum --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/Session.ice \
	move Session.h ..\..\include\glacier \
	

"..\..\include\glacier\Session.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Session.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\glacier\SessionF.ice

!IF  "$(CFG)" == "glacier - Win32 Release"

USERDEP__SESSIO="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier\SessionF.ice

"..\..\include\glacier\SessionF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --checksum --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/SessionF.ice 
	move SessionF.h ..\..\include\glacier 
	del SessionF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "glacier - Win32 Debug"

USERDEP__SESSIO="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier\SessionF.ice

"..\..\include\glacier\SessionF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --checksum --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/SessionF.ice 
	move SessionF.h ..\..\include\glacier 
	del SessionF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\glacier\SessionManager.ice

!IF  "$(CFG)" == "glacier - Win32 Release"

USERDEP__SESSION="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier\SessionManager.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --checksum --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/SessionManager.ice \
	move SessionManager.h ..\..\include\glacier \
	

"..\..\include\glacier\SessionManager.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"SessionManager.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "glacier - Win32 Debug"

USERDEP__SESSION="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier\SessionManager.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --checksum --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/SessionManager.ice \
	move SessionManager.h ..\..\include\glacier \
	

"..\..\include\glacier\SessionManager.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"SessionManager.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\glacier\SessionManagerF.ice

!IF  "$(CFG)" == "glacier - Win32 Release"

USERDEP__SESSIONM="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier\SessionManagerF.ice

"..\..\include\glacier\SessionManagerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --checksum --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/SessionManagerF.ice 
	move SessionManagerF.h ..\..\include\glacier 
	del SessionManagerF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "glacier - Win32 Debug"

USERDEP__SESSIONM="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier\SessionManagerF.ice

"..\..\include\glacier\SessionManagerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --checksum --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/SessionManagerF.ice 
	move SessionManagerF.h ..\..\include\glacier 
	del SessionManagerF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\glacier\Starter.ice

!IF  "$(CFG)" == "glacier - Win32 Release"

USERDEP__START="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier\Starter.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --checksum --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/Starter.ice \
	move Starter.h ..\..\include\glacier \
	

"..\..\include\glacier\Starter.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Starter.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "glacier - Win32 Debug"

USERDEP__START="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier\Starter.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --checksum --dll-export GLACIER_API --include-dir Glacier -I../../slice ../../slice/Glacier/Starter.ice \
	move Starter.h ..\..\include\glacier \
	

"..\..\include\glacier\Starter.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Starter.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
