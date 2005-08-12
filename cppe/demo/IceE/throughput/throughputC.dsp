# Microsoft Developer Studio Project File - Name="throughputC" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=throughputC - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "throughputC.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "throughputC.mak" CFG="throughputC - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "throughputC - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "throughputC - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "throughputC - Win32 Debug Static" (based on "Win32 (x86) Console Application")
!MESSAGE "throughputC - Win32 Release Static" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "throughputC - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseC"
# PROP Intermediate_Dir "ReleaseC"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I "." /I "../../../include" /D "NDEBUG" /D "WIN32_LEAN_AND_MEAN" /D "_CONSOLE" /D "ICEE_PURE_CLIENT" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 /nologo /subsystem:console /incremental:yes /machine:I386 /out:"client.exe" /libpath:"../../../lib"
# SUBTRACT LINK32 /debug /nodefaultlib

!ELSEIF  "$(CFG)" == "throughputC - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugC"
# PROP Intermediate_Dir "DebugC"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I "." /I "../../../include" /D "_DEBUG" /D "WIN32_LEAN_AND_MEAN" /D "_CONSOLE" /D "ICEE_PURE_CLIENT" /FD /GZ /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:console /debug /machine:I386 /out:"client.exe" /pdbtype:sept /libpath:"../../../lib" /fixed:no
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "throughputC - Win32 Debug Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "throughputC___Win32_Debug_Static"
# PROP BASE Intermediate_Dir "throughputC___Win32_Debug_Static"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugStaticC"
# PROP Intermediate_Dir "DebugStaticC"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I "." /I "../../../include" /I "../../../include/stlport" /D "_DEBUG" /D "_CONSOLE" /FD /GZ /c
# SUBTRACT BASE CPP /Fr /YX
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I "." /I "../../../include" /D "_DEBUG" /D "ICE_STATIC_LIBS" /D "WIN32_LEAN_AND_MEAN" /D "_CONSOLE" /D "ICEE_PURE_CLIENT" /FD /GZ /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 iceEd.lib /nologo /subsystem:console /debug /machine:I386 /out:"client.exe" /pdbtype:sept /libpath:"../../../lib"
# SUBTRACT BASE LINK32 /incremental:no /nodefaultlib
# ADD LINK32 ws2_32.lib rpcrt4.lib /nologo /subsystem:console /debug /machine:I386 /out:"client.exe" /pdbtype:sept /libpath:"../../../lib" /fixed:no
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "throughputC - Win32 Release Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "throughputC___Win32_Release_Static"
# PROP BASE Intermediate_Dir "throughputC___Win32_Release_Static"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseStaticC"
# PROP Intermediate_Dir "ReleaseStaticC"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /WX /GR /GX /O2 /I "." /I "../../../include" /I "../../../include/stlport" /D "NDEBUG" /D "_CONSOLE" /FD /c
# SUBTRACT BASE CPP /Fr /YX
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I "." /I "../../../include" /D "NDEBUG" /D "ICE_STATIC_LIBS" /D "WIN32_LEAN_AND_MEAN" /D "_CONSOLE" /D "ICEE_PURE_CLIENT" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 iceE.lib /nologo /subsystem:console /incremental:yes /machine:I386 /out:"client.exe" /libpath:"../../../lib"
# SUBTRACT BASE LINK32 /debug /nodefaultlib
# ADD LINK32 icee.lib ws2_32.lib rpcrt4.lib /nologo /subsystem:console /machine:I386 /out:"client.exe" /libpath:"../../../lib"
# SUBTRACT LINK32 /incremental:yes /debug /nodefaultlib

!ENDIF 

# Begin Target

# Name "throughputC - Win32 Release"
# Name "throughputC - Win32 Debug"
# Name "throughputC - Win32 Debug Static"
# Name "throughputC - Win32 Release Static"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Client.cpp
# End Source File
# Begin Source File

SOURCE=.\Throughput.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Throughput.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Throughput.ice

!IF  "$(CFG)" == "throughputC - Win32 Release"

# Begin Custom Build
InputPath=.\Throughput.ice

BuildCmds= \
	slice2cppe.exe Throughput.ice

"Throughput.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Throughput.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "throughputC - Win32 Debug"

# Begin Custom Build
InputPath=.\Throughput.ice

BuildCmds= \
	slice2cppe.exe Throughput.ice

"Throughput.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Throughput.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "throughputC - Win32 Debug Static"

# Begin Custom Build
InputPath=.\Throughput.ice

BuildCmds= \
	slice2cppe.exe Throughput.ice

"Throughput.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Throughput.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "throughputC - Win32 Release Static"

# Begin Custom Build
InputPath=.\Throughput.ice

BuildCmds= \
	slice2cppe.exe Throughput.ice

"Throughput.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Throughput.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
