# Microsoft Developer Studio Project File - Name="glacier2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=glacier2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Glacier2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Glacier2.mak" CFG="glacier2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "glacier2 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "glacier2 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "glacier2 - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I ".." /I "../../include" /D "NDEBUG" /D "_USRDLL" /D "GLACIER2_API_EXPORTS" /D "_CONSOLE" /FD /c
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
# ADD LINK32 /nologo /dll /incremental:yes /machine:I386 /out:"Release/glacier221.dll" /implib:"Release/glacier2.lib"
# SUBTRACT LINK32 /pdb:none /debug /nodefaultlib
# Begin Special Build Tool
OutDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\glacier2.lib ..\..\lib	copy $(OutDir)\glacier221.dll ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "glacier2 - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "_DEBUG" /D "_USRDLL" /D "GLACIER2_API_EXPORTS" /D "_CONSOLE" /FD /GZ /c
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
# ADD LINK32 /nologo /dll /debug /machine:I386 /out:"Debug/glacier221d.dll" /implib:"Debug/glacier2d.lib" /FIXED:no
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
OutDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\glacier2d.lib ..\..\lib	copy $(OutDir)\glacier221d.pdb ..\..\bin	copy $(OutDir)\glacier221d.dll ..\..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "glacier2 - Win32 Release"
# Name "glacier2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\PermissionsVerifier.cpp
# End Source File
# Begin Source File

SOURCE=.\Router.cpp
# End Source File
# Begin Source File

SOURCE=.\Session.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\glacier2\PermissionsVerifier.h
# End Source File
# Begin Source File

SOURCE=..\..\include\glacier2\PermissionsVerifierF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\glacier2\Router.h
# End Source File
# Begin Source File

SOURCE=..\..\include\glacier2\RouterF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\glacier2\Session.h
# End Source File
# Begin Source File

SOURCE=..\..\include\glacier2\SessionF.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\slice\glacier2\PermissionsVerifier.ice

!IF  "$(CFG)" == "glacier2 - Win32 Release"

USERDEP__PERMI="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier2\PermissionsVerifier.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export GLACIER2_API --include-dir Glacier2 -I../../slice ../../slice/Glacier2/PermissionsVerifier.ice \
	move PermissionsVerifier.h ..\..\include\glacier2 \
	

"..\..\include\glacier2\PermissionsVerifier.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"PermissionsVerifier.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "glacier2 - Win32 Debug"

USERDEP__PERMI="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier2\PermissionsVerifier.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export GLACIER2_API --include-dir Glacier2 -I../../slice ../../slice/Glacier2/PermissionsVerifier.ice \
	move PermissionsVerifier.h ..\..\include\glacier2 \
	

"..\..\include\glacier2\PermissionsVerifier.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"PermissionsVerifier.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\glacier2\PermissionsVerifierF.ice

!IF  "$(CFG)" == "glacier2 - Win32 Release"

USERDEP__PERMIS="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier2\PermissionsVerifierF.ice

"..\..\include\glacier2\PermissionsVerifierF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --dll-export GLACIER2_API --include-dir Glacier2 -I../../slice ../../slice/Glacier2/PermissionsVerifierF.ice 
	move PermissionsVerifierF.h ..\..\include\glacier2 
	del PermissionsVerifierF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "glacier2 - Win32 Debug"

USERDEP__PERMIS="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier2\PermissionsVerifierF.ice

"..\..\include\glacier2\PermissionsVerifierF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --dll-export GLACIER2_API --include-dir Glacier2 -I../../slice ../../slice/Glacier2/PermissionsVerifierF.ice 
	move PermissionsVerifierF.h ..\..\include\glacier2 
	del PermissionsVerifierF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\glacier2\Router.ice

!IF  "$(CFG)" == "glacier2 - Win32 Release"

USERDEP__ROUTE="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier2\Router.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export GLACIER2_API --include-dir Glacier2 -I../../slice ../../slice/Glacier2/Router.ice \
	move Router.h ..\..\include\glacier2 \
	

"..\..\include\glacier2\Router.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Router.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "glacier2 - Win32 Debug"

USERDEP__ROUTE="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier2\Router.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export GLACIER2_API --include-dir Glacier2 -I../../slice ../../slice/Glacier2/Router.ice \
	move Router.h ..\..\include\glacier2 \
	

"..\..\include\glacier2\Router.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Router.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\glacier2\RouterF.ice

!IF  "$(CFG)" == "glacier2 - Win32 Release"

USERDEP__ROUTER="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier2\RouterF.ice

"..\..\include\glacier2\RouterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --dll-export GLACIER2_API --include-dir Glacier2 -I../../slice ../../slice/Glacier2/RouterF.ice 
	move RouterF.h ..\..\include\glacier2 
	del RouterF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "glacier2 - Win32 Debug"

USERDEP__ROUTER="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier2\RouterF.ice

"..\..\include\glacier2\RouterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --dll-export GLACIER2_API --include-dir Glacier2 -I../../slice ../../slice/Glacier2/RouterF.ice 
	move RouterF.h ..\..\include\glacier2 
	del RouterF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\glacier2\Session.ice

!IF  "$(CFG)" == "glacier2 - Win32 Release"

USERDEP__SESSI="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier2\Session.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export GLACIER2_API --include-dir Glacier2 -I../../slice ../../slice/Glacier2/Session.ice \
	move Session.h ..\..\include\glacier2 \
	

"..\..\include\glacier2\Session.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Session.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "glacier2 - Win32 Debug"

USERDEP__SESSI="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier2\Session.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export GLACIER2_API --include-dir Glacier2 -I../../slice ../../slice/Glacier2/Session.ice \
	move Session.h ..\..\include\glacier2 \
	

"..\..\include\glacier2\Session.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Session.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\glacier2\SessionF.ice

!IF  "$(CFG)" == "glacier2 - Win32 Release"

USERDEP__SESSIO="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier2\SessionF.ice

"..\..\include\glacier2\SessionF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --dll-export GLACIER2_API --include-dir Glacier2 -I../../slice ../../slice/Glacier2/SessionF.ice 
	move SessionF.h ..\..\include\glacier2 
	del SessionF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "glacier2 - Win32 Debug"

USERDEP__SESSIO="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\glacier2\SessionF.ice

"..\..\include\glacier2\SessionF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --dll-export GLACIER2_API --include-dir Glacier2 -I../../slice ../../slice/Glacier2/SessionF.ice 
	move SessionF.h ..\..\include\glacier2 
	del SessionF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
