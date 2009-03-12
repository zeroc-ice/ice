# Microsoft Developer Studio Project File - Name="Freeze.casino.server.exe" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Freeze.casino.server.exe - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Freeze.casino.server.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Freeze.casino.server.mak" CFG="Freeze.casino.server.exe - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Freeze.casino.server.exe - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Freeze.casino.server.exe - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Freeze.casino.server.exe - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I "." /I "../../../include" /I "../../../include/stlport" /I "dummyinclude" /D "_CONSOLE" /D "NDEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 IceXML.lib Freeze.lib Ice.lib IceUtil.lib setargv.obj /nologo /subsystem:console /pdb:none /machine:I386 /out:"server.exe" /libpath:"../../../lib" /FIXED:no /IGNORE:4089
# SUBTRACT LINK32 /debug /nodefaultlib

!ELSEIF  "$(CFG)" == "Freeze.casino.server.exe - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I "." /I "../../../include" /I "../../../include/stlport" /I "dummyinclude" /D "_CONSOLE" /D "_DEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /GZ /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Freezed.lib Iced.lib IceUtild.lib setargv.obj /nologo /subsystem:console /debug /machine:I386 /out:"server.exe" /pdbtype:sept /libpath:"../../../lib" /FIXED:no
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Freeze.casino.server.exe - Win32 Release"
# Name "Freeze.casino.server.exe - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BankI.cpp
# End Source File
# Begin Source File

SOURCE=.\BetI.cpp
# End Source File
# Begin Source File

SOURCE=.\BetResolver.cpp
# End Source File
# Begin Source File

SOURCE=.\Casino.cpp
# End Source File
# Begin Source File

SOURCE=.\CasinoStore.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerI.cpp
# End Source File
# Begin Source File

SOURCE=.\Server.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BankI.h
# End Source File
# Begin Source File

SOURCE=.\BetI.h
# End Source File
# Begin Source File

SOURCE=.\BetResolver.h
# End Source File
# Begin Source File

SOURCE=.\Casino.h
# End Source File
# Begin Source File

SOURCE=.\CasinoStore.h
# End Source File
# Begin Source File

SOURCE=.\PlayerI.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Casino.ice

!IF  "$(CFG)" == "Freeze.casino.server.exe - Win32 Release"

USERDEP__CASIN="..\..\..\bin\slice2cpp.exe"	"..\..\..\bin\slice2freeze.exe"	"..\..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=.\Casino.ice

BuildCmds= \
	..\..\..\bin\slice2cpp.exe Casino.ice

"Casino.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Casino.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Freeze.casino.server.exe - Win32 Debug"

USERDEP__CASIN="..\..\..\bin\slice2cpp.exe"	"..\..\..\bin\slice2freeze.exe"	"..\..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=.\Casino.ice

BuildCmds= \
	..\..\..\bin\slice2cpp.exe Casino.ice

"Casino.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Casino.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CasinoStore.ice

!IF  "$(CFG)" == "Freeze.casino.server.exe - Win32 Release"

# Begin Custom Build
InputPath=.\CasinoStore.ice

BuildCmds= \
	if exist ../../../slice (set sliceDir=../../../slice) else set sliceDir=../../../../slice \
	..\..\..\bin\slice2cpp.exe -I. -I%slicedir% CasinoStore.ice \
	

"CasinoStore.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"CasinoStore.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Freeze.casino.server.exe - Win32 Debug"

# Begin Custom Build
InputPath=.\CasinoStore.ice

BuildCmds= \
	if exist ../../../slice (set sliceDir=../../../slice) else set sliceDir=../../../../slice \
	..\..\..\bin\slice2cpp.exe -I. -I%slicedir% CasinoStore.ice \
	

"CasinoStore.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"CasinoStore.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\README
# End Source File
# End Target
# End Project
