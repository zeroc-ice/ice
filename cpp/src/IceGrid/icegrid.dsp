# Microsoft Developer Studio Project File - Name="icegrid" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=icegrid - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "icegrid.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "icegrid.mak" CFG="icegrid - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "icegrid - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "icegrid - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "icegrid - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I ".." /I "../../include" /D "_USRDLL" /D "ICE_GRID_API_EXPORTS" /D "_CONSOLE" /D "NDEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /c
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
# ADD LINK32 /nologo /dll /incremental:yes /machine:I386 /out:"Release/icegrid30.dll" /implib:"Release/icegrid.lib" /FIXED:no
# SUBTRACT LINK32 /pdb:none /debug /nodefaultlib
# Begin Special Build Tool
OutDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\icegrid.lib ..\..\lib	copy $(OutDir)\icegrid30.dll ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "icegrid - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "_USRDLL" /D "ICE_GRID_API_EXPORTS" /D "_CONSOLE" /D "_DEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /GZ /c
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
# ADD LINK32 /nologo /dll /debug /machine:I386 /out:"Debug/icegrid30d.dll" /implib:"Debug/icegridd.lib" /FIXED:no
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
OutDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\icegridd.lib ..\..\lib	copy $(OutDir)\icegrid30d.pdb ..\..\bin	copy $(OutDir)\icegrid30d.dll ..\..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "icegrid - Win32 Release"
# Name "icegrid - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Admin.cpp
# End Source File
# Begin Source File

SOURCE=.\Descriptor.cpp
# End Source File
# Begin Source File

SOURCE=.\Exception.cpp
# End Source File
# Begin Source File

SOURCE=.\Observer.cpp
# End Source File
# Begin Source File

SOURCE=.\Query.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\icegrid\Admin.h
# End Source File
# Begin Source File

SOURCE=..\..\include\icegrid\Exception.h
# End Source File
# Begin Source File

SOURCE=..\..\include\icegrid\Query.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\slice\icegrid\Admin.ice

!IF  "$(CFG)" == "icegrid - Win32 Release"

USERDEP__ADMIN="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\icegrid\Admin.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --checksum --dll-export ICE_GRID_API --include-dir icegrid -I../../slice ../../slice/IceGrid/Admin.ice \
	move Admin.h ..\..\include\icegrid \
	

"..\..\include\icegrid\Admin.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Admin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icegrid - Win32 Debug"

USERDEP__ADMIN="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\icegrid\Admin.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --checksum --dll-export ICE_GRID_API --include-dir icegrid -I../../slice ../../slice/IceGrid/Admin.ice \
	move Admin.h ..\..\include\icegrid \
	

"..\..\include\icegrid\Admin.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Admin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\icegrid\Descriptor.ice

!IF  "$(CFG)" == "icegrid - Win32 Release"

USERDEP__DESCR="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\icegrid\Descriptor.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --checksum --dll-export ICE_GRID_API --include-dir icegrid -I../../slice ../../slice/IceGrid/Descriptor.ice \
	move Descriptor.h ..\..\include\icegrid \
	

"..\..\include\icegrid\Descriptor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Descriptor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icegrid - Win32 Debug"

USERDEP__DESCR="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\icegrid\Descriptor.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --checksum --dll-export ICE_GRID_API --include-dir icegrid -I../../slice ../../slice/IceGrid/Descriptor.ice \
	move Descriptor.h ..\..\include\icegrid \
	

"..\..\include\icegrid\Descriptor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Descriptor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\icegrid\Exception.ice

!IF  "$(CFG)" == "icegrid - Win32 Release"

USERDEP__EXCEP="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\icegrid\Exception.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --checksum --dll-export ICE_GRID_API --include-dir icegrid -I../../slice ../../slice/IceGrid/Exception.ice \
	move Exception.h ..\..\include\icegrid \
	

"..\..\include\icegrid\Exception.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Exception.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icegrid - Win32 Debug"

USERDEP__EXCEP="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\icegrid\Exception.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --checksum --dll-export ICE_GRID_API --include-dir icegrid -I../../slice ../../slice/IceGrid/Exception.ice \
	move Exception.h ..\..\include\icegrid \
	

"..\..\include\icegrid\Exception.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Exception.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\icegrid\Observer.ice

!IF  "$(CFG)" == "icegrid - Win32 Release"

USERDEP__OBSER="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\icegrid\Observer.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --checksum --dll-export ICE_GRID_API --include-dir icegrid -I../../slice ../../slice/IceGrid/Observer.ice \
	move Observer.h ..\..\include\icegrid \
	

"..\..\include\icegrid\Observer.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Observer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icegrid - Win32 Debug"

USERDEP__OBSER="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\icegrid\Observer.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --checksum --dll-export ICE_GRID_API --include-dir icegrid -I../../slice ../../slice/IceGrid/Observer.ice \
	move Observer.h ..\..\include\icegrid \
	

"..\..\include\icegrid\Observer.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Observer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\icegrid\Query.ice

!IF  "$(CFG)" == "icegrid - Win32 Release"

USERDEP__QUERY="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\icegrid\Query.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --checksum --dll-export ICE_GRID_API --include-dir icegrid -I../../slice ../../slice/IceGrid/Query.ice \
	move Query.h ..\..\include\icegrid \
	

"..\..\include\icegrid\Query.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Query.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icegrid - Win32 Debug"

USERDEP__QUERY="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\icegrid\Query.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --checksum --dll-export ICE_GRID_API --include-dir icegrid -I../../slice ../../slice/IceGrid/Query.ice \
	move Query.h ..\..\include\icegrid \
	

"..\..\include\icegrid\Query.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Query.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
