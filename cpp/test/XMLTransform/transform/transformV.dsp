# Microsoft Developer Studio Project File - Name="transformV" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=transformV - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "transformV.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "transformV.mak" CFG="transformV - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "transformV - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "transformV - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "transformV - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I "." /I "../../../include" /I "../../include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_UNICODE" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 /nologo /subsystem:console /machine:I386 /out:"validate.exe" /libpath:"../../../lib"
# SUBTRACT LINK32 /debug /nodefaultlib

!ELSEIF  "$(CFG)" == "transformV - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I "." /I "../../../include" /I "../../include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_UNICODE" /FD /GZ /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:console /debug /machine:I386 /out:"validate.exe" /pdbtype:sept /libpath:"../../../lib"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "transformV - Win32 Release"
# Name "transformV - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\IntByteMap.cpp
# End Source File
# Begin Source File

SOURCE=.\IntC1MapNew.cpp
# End Source File
# Begin Source File

SOURCE=.\IntD1MapNew.cpp
# End Source File
# Begin Source File

SOURCE=.\IntDoubleMap.cpp
# End Source File
# Begin Source File

SOURCE=.\IntE1MapNew.cpp
# End Source File
# Begin Source File

SOURCE=.\IntFloatMap.cpp
# End Source File
# Begin Source File

SOURCE=.\IntIntMap.cpp
# End Source File
# Begin Source File

SOURCE=.\IntLongMap.cpp
# End Source File
# Begin Source File

SOURCE=.\IntS1MapNew.cpp
# End Source File
# Begin Source File

SOURCE=.\IntSeq1MapNew.cpp
# End Source File
# Begin Source File

SOURCE=.\IntSeq2MapNew.cpp
# End Source File
# Begin Source File

SOURCE=.\IntSeq3MapNew.cpp
# End Source File
# Begin Source File

SOURCE=.\IntSeq4MapNew.cpp
# End Source File
# Begin Source File

SOURCE=.\IntShortMap.cpp
# End Source File
# Begin Source File

SOURCE=.\TestNew.cpp
# End Source File
# Begin Source File

SOURCE=.\Validate.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\IntByteMap.h
# End Source File
# Begin Source File

SOURCE=.\IntC1MapNew.h
# End Source File
# Begin Source File

SOURCE=.\IntD1MapNew.h
# End Source File
# Begin Source File

SOURCE=.\IntDoubleMap.h
# End Source File
# Begin Source File

SOURCE=.\IntE1MapNew.h
# End Source File
# Begin Source File

SOURCE=.\IntFloatMap.h
# End Source File
# Begin Source File

SOURCE=.\IntIntMap.h
# End Source File
# Begin Source File

SOURCE=.\IntLongMap.h
# End Source File
# Begin Source File

SOURCE=.\IntS1MapNew.h
# End Source File
# Begin Source File

SOURCE=.\IntSeq1MapNew.h
# End Source File
# Begin Source File

SOURCE=.\IntSeq2MapNew.h
# End Source File
# Begin Source File

SOURCE=.\IntSeq3MapNew.h
# End Source File
# Begin Source File

SOURCE=.\IntSeq4MapNew.h
# End Source File
# Begin Source File

SOURCE=.\IntShortMap.h
# End Source File
# Begin Source File

SOURCE=.\TestNew.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\TestNew.ice

!IF  "$(CFG)" == "transformV - Win32 Release"

USERDEP__TESTN="..\..\..\bin\slice2cpp.exe"	"..\..\..\bin\slice2freeze.exe"	
# Begin Custom Build
InputPath=.\TestNew.ice

BuildCmds= \
	..\..\..\bin\slice2cpp.exe TestNew.ice \
	..\..\..\bin\slice2freeze.exe --dict IntSeq1Map,int,Test::Seq1 IntSeq1MapNew TestNew.ice \
	..\..\..\bin\slice2freeze.exe --dict IntSeq2Map,int,Test::Seq2 IntSeq2MapNew TestNew.ice \
	..\..\..\bin\slice2freeze.exe --dict IntSeq3Map,int,Test::Seq3 IntSeq3MapNew TestNew.ice \
	..\..\..\bin\slice2freeze.exe --dict IntSeq4Map,int,Test::Seq4 IntSeq4MapNew TestNew.ice \
	..\..\..\bin\slice2freeze.exe --dict IntE1Map,int,Test::E1 IntE1MapNew TestNew.ice \
	..\..\..\bin\slice2freeze.exe --dict IntD1Map,int,Test::D1 IntD1MapNew TestNew.ice \
	..\..\..\bin\slice2freeze.exe --dict IntS1Map,int,Test::S1 IntS1MapNew TestNew.ice \
	..\..\..\bin\slice2freeze.exe --dict IntC1Map,int,Test::C1 IntC1MapNew TestNew.ice \
	

"TestNew.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestNew.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq1MapNew.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq1MapNew.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq2MapNew.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq2MapNew.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq3MapNew.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq3MapNew.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq4MapNew.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq4MapNew.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntE1MapNew.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntE1MapNew.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntD1MapNew.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntD1MapNew.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntS1MapNew.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntS1MapNew.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntC1MapNew.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntC1MapNew.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "transformV - Win32 Debug"

USERDEP__TESTN="..\..\..\bin\slice2cpp.exe"	"..\..\..\bin\slice2freeze.exe"	
# Begin Custom Build
InputPath=.\TestNew.ice

BuildCmds= \
	..\..\..\bin\slice2cpp.exe TestNew.ice \
	..\..\..\bin\slice2freeze.exe --dict IntSeq1Map,int,Test::Seq1 IntSeq1MapNew TestNew.ice \
	..\..\..\bin\slice2freeze.exe --dict IntSeq2Map,int,Test::Seq2 IntSeq2MapNew TestNew.ice \
	..\..\..\bin\slice2freeze.exe --dict IntSeq3Map,int,Test::Seq3 IntSeq3MapNew TestNew.ice \
	..\..\..\bin\slice2freeze.exe --dict IntSeq4Map,int,Test::Seq4 IntSeq4MapNew TestNew.ice \
	..\..\..\bin\slice2freeze.exe --dict IntE1Map,int,Test::E1 IntE1MapNew TestNew.ice \
	..\..\..\bin\slice2freeze.exe --dict IntD1Map,int,Test::D1 IntD1MapNew TestNew.ice \
	..\..\..\bin\slice2freeze.exe --dict IntS1Map,int,Test::S1 IntS1MapNew TestNew.ice \
	..\..\..\bin\slice2freeze.exe --dict IntC1Map,int,Test::C1 IntC1MapNew TestNew.ice \
	

"TestNew.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestNew.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq1MapNew.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq1MapNew.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq2MapNew.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq2MapNew.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq3MapNew.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq3MapNew.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq4MapNew.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq4MapNew.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntE1MapNew.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntE1MapNew.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntD1MapNew.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntD1MapNew.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntS1MapNew.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntS1MapNew.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntC1MapNew.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntC1MapNew.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
