# Microsoft Developer Studio Project File - Name="IcePackS" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=IcePackS - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "IcePackS.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "IcePackS.mak" CFG="IcePackS - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "IcePackS - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "IcePackS - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "IcePackS - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I ".." /I "../../include" /D "NDEBUG" /D "_CONSOLE" /D "_UNICODE" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 setargv.obj /nologo /subsystem:console /machine:I386 /out:"../../bin/icepack.exe" /libpath:"../../../lib"
# SUBTRACT LINK32 /debug /nodefaultlib

!ELSEIF  "$(CFG)" == "IcePackS - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "_DEBUG" /D "_CONSOLE" /D "_UNICODE" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 setargv.obj /nologo /subsystem:console /debug /machine:I386 /out:"../../bin/icepack.exe" /pdbtype:sept /libpath:"../../../lib"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "IcePackS - Win32 Release"
# Name "IcePackS - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Activator.cpp
# End Source File
# Begin Source File

SOURCE=.\AdapterManager.cpp
# End Source File
# Begin Source File

SOURCE=.\AdapterManagerI.cpp
# End Source File
# Begin Source File

SOURCE=.\AdminI.cpp
# End Source File
# Begin Source File

SOURCE=.\LocatorI.cpp
# End Source File
# Begin Source File

SOURCE=.\LocatorRegistryI.cpp
# End Source File
# Begin Source File

SOURCE=.\Server.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerManagerI.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Activator.h
# End Source File
# Begin Source File

SOURCE=.\AdapterManager.h
# End Source File
# Begin Source File

SOURCE=.\AdapterManagerF.h
# End Source File
# Begin Source File

SOURCE=.\AdapterManagerI.h
# End Source File
# Begin Source File

SOURCE=.\AdminI.h
# End Source File
# Begin Source File

SOURCE=.\LocatorI.h
# End Source File
# Begin Source File

SOURCE=.\LocatorRegistryI.h
# End Source File
# Begin Source File

SOURCE=.\ServerManager.h
# End Source File
# Begin Source File

SOURCE=.\ServerManagerF.h
# End Source File
# Begin Source File

SOURCE=.\ServerManagerI.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Activator.ice

!IF  "$(CFG)" == "IcePackS - Win32 Release"

USERDEP__ACTIV="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=.\Activator.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --include-dir IcePack -I../../slice -I.. Activator.ice \
	

"Activator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Activator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IcePackS - Win32 Debug"

USERDEP__ACTIV="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=.\Activator.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --include-dir IcePack -I../../slice -I.. Activator.ice \
	

"Activator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Activator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AdapterManager.ice

!IF  "$(CFG)" == "IcePackS - Win32 Release"

USERDEP__ADAPT="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=.\AdapterManager.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --include-dir IcePack -I../../slice -I.. AdapterManager.ice \
	

"AdapterManager.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"AdapterManager.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IcePackS - Win32 Debug"

USERDEP__ADAPT="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=.\AdapterManager.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --include-dir IcePack -I../../slice -I.. AdapterManager.ice \
	

"AdapterManager.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"AdapterManager.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AdapterManagerF.ice

!IF  "$(CFG)" == "IcePackS - Win32 Release"

USERDEP__ADAPTE="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=.\AdapterManagerF.ice

"AdapterManagerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --include-dir IcePack -I../../slice -I.. AdapterManagerF.ice 
	del AdapterManagerF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "IcePackS - Win32 Debug"

USERDEP__ADAPTE="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=.\AdapterManagerF.ice

"AdapterManagerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --include-dir IcePack -I../../slice -I.. AdapterManagerF.ice 
	del AdapterManagerF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ServerManager.ice

!IF  "$(CFG)" == "IcePackS - Win32 Release"

USERDEP__SERVE="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=.\ServerManager.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --include-dir IcePack -I../../slice -I.. ServerManager.ice \
	

"ServerManager.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ServerManager.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IcePackS - Win32 Debug"

USERDEP__SERVE="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=.\ServerManager.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --include-dir IcePack -I../../slice -I.. ServerManager.ice \
	

"ServerManager.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ServerManager.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ServerManagerF.ice

!IF  "$(CFG)" == "IcePackS - Win32 Release"

USERDEP__SERVER="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=.\ServerManagerF.ice

"ServerManagerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --include-dir IcePack -I../../slice -I.. ServerManagerF.ice 
	del ServerManagerF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "IcePackS - Win32 Debug"

USERDEP__SERVER="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=.\ServerManagerF.ice

"ServerManagerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --include-dir IcePack -I../../slice -I.. ServerManagerF.ice 
	del ServerManagerF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
