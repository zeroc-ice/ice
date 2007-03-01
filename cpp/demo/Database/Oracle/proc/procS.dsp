# Microsoft Developer Studio Project File - Name="procS" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=procS - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "procS.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "procS.mak" CFG="procS - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "procS - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "procS - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "procS - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I "." /I "../../../../include" /I "../../../../include/stlport" /D "_CONSOLE" /D "NDEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 orasql10.lib Ice.lib IceUtil.lib setargv.obj /nologo /subsystem:console /pdb:none /machine:I386 /out:"server.exe" /libpath:"../../../../lib" /FIXED:no
# SUBTRACT LINK32 /debug /nodefaultlib

!ELSEIF  "$(CFG)" == "procS - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I "." /I "../../../../include" /I "../../../../include/stlport" /D "_CONSOLE" /D "_DEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /GZ /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 orasql10.lib Iced.lib IceUtild.lib setargv.obj /nologo /subsystem:console /debug /machine:I386 /out:"server.exe" /pdbtype:sept /libpath:"../../../../lib" /FIXED:no
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "procS - Win32 Release"
# Name "procS - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CurrentSqlContext.cpp
# ADD CPP /D "SQLCA_NONE"
# End Source File
# Begin Source File

SOURCE=.\DeptFactoryI.cpp
# ADD CPP /D "SQLCA_NONE"
# End Source File
# Begin Source File

SOURCE=.\DeptI.cpp
# ADD CPP /D "SQLCA_NONE"
# End Source File
# Begin Source File

SOURCE=.\EmpI.cpp
# ADD CPP /D "SQLCA_NONE"
# End Source File
# Begin Source File

SOURCE=.\HR.cpp
# End Source File
# Begin Source File

SOURCE=.\Server.cpp
# ADD CPP /D "SQLCA_NONE"
# End Source File
# Begin Source File

SOURCE=.\Util.cpp
# ADD CPP /D "SQLCA_NONE"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CurrentSqlContext.h
# End Source File
# Begin Source File

SOURCE=.\DeptFactoryI.h
# End Source File
# Begin Source File

SOURCE=.\DeptI.h
# End Source File
# Begin Source File

SOURCE=.\EmpI.h
# End Source File
# Begin Source File

SOURCE=.\HR.h
# End Source File
# Begin Source File

SOURCE=.\Util.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\CurrentSqlContext.pc

!IF  "$(CFG)" == "procS - Win32 Release"

# Begin Custom Build
InputPath=.\CurrentSqlContext.pc
InputName=CurrentSqlContext

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	proc threads=yes parse=none lines=yes code=cpp cpp_suffix=cpp close_on_commit=yes $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "procS - Win32 Debug"

# Begin Custom Build
InputPath=.\CurrentSqlContext.pc
InputName=CurrentSqlContext

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	proc threads=yes parse=none lines=yes code=cpp cpp_suffix=cpp close_on_commit=yes $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DeptFactoryI.pc

!IF  "$(CFG)" == "procS - Win32 Release"

# Begin Custom Build
InputPath=.\DeptFactoryI.pc
InputName=DeptFactoryI

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	proc threads=yes parse=none lines=yes code=cpp cpp_suffix=cpp close_on_commit=yes $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "procS - Win32 Debug"

# Begin Custom Build
InputPath=.\DeptFactoryI.pc
InputName=DeptFactoryI

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	proc threads=yes parse=none lines=yes code=cpp cpp_suffix=cpp close_on_commit=yes $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DeptI.pc

!IF  "$(CFG)" == "procS - Win32 Release"

# Begin Custom Build
InputPath=.\DeptI.pc
InputName=DeptI

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	proc threads=yes parse=none lines=yes code=cpp cpp_suffix=cpp close_on_commit=yes $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "procS - Win32 Debug"

# Begin Custom Build
InputPath=.\DeptI.pc
InputName=DeptI

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	proc threads=yes parse=none lines=yes code=cpp cpp_suffix=cpp close_on_commit=yes $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\EmpI.pc

!IF  "$(CFG)" == "procS - Win32 Release"

# Begin Custom Build
InputPath=.\EmpI.pc
InputName=EmpI

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	proc threads=yes parse=none lines=yes code=cpp cpp_suffix=cpp close_on_commit=yes $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "procS - Win32 Debug"

# Begin Custom Build
InputPath=.\EmpI.pc
InputName=EmpI

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	proc threads=yes parse=none lines=yes code=cpp cpp_suffix=cpp close_on_commit=yes $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HR.ice

!IF  "$(CFG)" == "procS - Win32 Release"

USERDEP__HR_IC="..\..\..\..\bin\slice2cpp.exe"	"..\..\..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=.\HR.ice

BuildCmds= \
	..\..\..\..\bin\slice2cpp.exe HR.ice

"HR.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"HR.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "procS - Win32 Debug"

USERDEP__HR_IC="..\..\..\..\bin\slice2cpp.exe"	"..\..\..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=.\HR.ice

BuildCmds= \
	..\..\..\..\bin\slice2cpp.exe HR.ice

"HR.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"HR.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Server.pc

!IF  "$(CFG)" == "procS - Win32 Release"

# Begin Custom Build
InputPath=.\Server.pc
InputName=Server

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	proc threads=yes parse=none lines=yes code=cpp cpp_suffix=cpp close_on_commit=yes $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "procS - Win32 Debug"

# Begin Custom Build
InputPath=.\Server.pc
InputName=Server

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	proc threads=yes parse=none lines=yes code=cpp cpp_suffix=cpp close_on_commit=yes $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Util.pc

!IF  "$(CFG)" == "procS - Win32 Release"

# Begin Custom Build
InputPath=.\Util.pc
InputName=Util

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	proc threads=yes parse=none lines=yes code=cpp cpp_suffix=cpp close_on_commit=yes $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "procS - Win32 Debug"

# Begin Custom Build
InputPath=.\Util.pc
InputName=Util

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	proc threads=yes parse=none lines=yes code=cpp cpp_suffix=cpp close_on_commit=yes $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\README
# End Source File
# End Target
# End Project
