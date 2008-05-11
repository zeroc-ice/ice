# Microsoft Developer Studio Project File - Name="sliceexAMDS" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=sliceexAMDS - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sliceexAMDS.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sliceexAMDS.mak" CFG="sliceexAMDS - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sliceexAMDS - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "sliceexAMDS - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sliceexAMDS - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I "." /I "../../../../include" /I "../../../include" /D "_CONSOLE" /D "NDEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /c
# SUBTRACT CPP /Z<none> /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 /nologo /subsystem:console /pdb:none /machine:I386 /out:"serveramd.exe" /libpath:"../../../../lib" /FIXED:no
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "sliceexAMDS - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I "." /I "../../../../include" /I "../../../include" /D "_CONSOLE" /D "_DEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:console /debug /machine:I386 /out:"serveramd.exe" /pdbtype:sept /libpath:"../../../../lib" /FIXED:no
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "sliceexAMDS - Win32 Release"
# Name "sliceexAMDS - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ServerAMD.cpp

!IF  "$(CFG)" == "sliceexAMDS - Win32 Release"

!ELSEIF  "$(CFG)" == "sliceexAMDS - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ServerPrivateAMD.cpp
# End Source File
# Begin Source File

SOURCE=.\TestAMD.cpp
# End Source File
# Begin Source File

SOURCE=.\TestAMDI.cpp

!IF  "$(CFG)" == "sliceexAMDS - Win32 Release"

!ELSEIF  "$(CFG)" == "sliceexAMDS - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ServerPrivateAMD.h
# End Source File
# Begin Source File

SOURCE=.\TestAMD.h
# End Source File
# Begin Source File

SOURCE=.\TestAMDI.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\ServerPrivateAMD.ice

!IF  "$(CFG)" == "sliceexAMDS - Win32 Release"

USERDEP__SERVE="..\..\..\..\bin\slice2cpp.exe"	"TestAMD.ice"	"..\..\..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=.\ServerPrivateAMD.ice

BuildCmds= \
	..\..\..\..\bin\slice2cpp.exe -I. ServerPrivateAMD.ice

"ServerPrivateAMD.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ServerPrivateAMD.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sliceexAMDS - Win32 Debug"

USERDEP__SERVE="..\..\..\..\bin\slice2cpp.exe"	"TestAMD.ice"	"..\..\..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=.\ServerPrivateAMD.ice

BuildCmds= \
	..\..\..\..\bin\slice2cpp.exe -I. ServerPrivateAMD.ice

"ServerPrivateAMD.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ServerPrivateAMD.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TestAMD.ice

!IF  "$(CFG)" == "sliceexAMDS - Win32 Release"

USERDEP__TESTA="..\..\..\..\bin\slice2cpp.exe"	"..\..\..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=.\TestAMD.ice

BuildCmds= \
	..\..\..\..\bin\slice2cpp.exe TestAMD.ice

"TestAMD.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestAMD.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sliceexAMDS - Win32 Debug"

USERDEP__TESTA="..\..\..\..\bin\slice2cpp.exe"	"..\..\..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=.\TestAMD.ice

BuildCmds= \
	..\..\..\..\bin\slice2cpp.exe TestAMD.ice

"TestAMD.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestAMD.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
