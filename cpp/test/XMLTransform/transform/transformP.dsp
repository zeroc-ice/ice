# Microsoft Developer Studio Project File - Name="transformP" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=transformP - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "transformP.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "transformP.mak" CFG="transformP - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "transformP - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "transformP - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "transformP - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I "." /I "../../../include" /I "../../include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_UNICODE" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 /nologo /subsystem:console /machine:I386 /out:"populate.exe" /libpath:"../../../lib"
# SUBTRACT LINK32 /debug /nodefaultlib

!ELSEIF  "$(CFG)" == "transformP - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I "." /I "../../../include" /I "../../include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_UNICODE" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:console /debug /machine:I386 /out:"populate.exe" /pdbtype:sept /libpath:"../../../lib"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "transformP - Win32 Release"
# Name "transformP - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Populate.cpp
# End Source File
# Begin Source File

SOURCE=.\TestOld.cpp
# End Source File
# Begin Source File

SOURCE=.\IntByteMap.cpp
# End Source File
# Begin Source File

SOURCE=.\IntShortMap.cpp
# End Source File
# Begin Source File

SOURCE=.\IntIntMap.cpp
# End Source File
# Begin Source File

SOURCE=.\IntLongMap.cpp
# End Source File
# Begin Source File

SOURCE=.\IntFloatMap.cpp
# End Source File
# Begin Source File

SOURCE=.\IntDoubleMap.cpp
# End Source File
# Begin Source File

SOURCE=.\IntSeq1MapOld.cpp
# End Source File
# Begin Source File

SOURCE=.\IntSeq2MapOld.cpp
# End Source File
# Begin Source File

SOURCE=.\IntSeq3MapOld.cpp
# End Source File
# Begin Source File

SOURCE=.\IntSeq4MapOld.cpp
# End Source File
# Begin Source File

SOURCE=.\IntE1MapOld.cpp
# End Source File
# Begin Source File

SOURCE=.\IntD1MapOld.cpp
# End Source File
# Begin Source File

SOURCE=.\IntS1MapOld.cpp
# End Source File
# Begin Source File

SOURCE=.\IntC1MapOld.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\TestOld.h
# End Source File
# Begin Source File

SOURCE=.\IntByteMap.h
# End Source File
# Begin Source File

SOURCE=.\IntShortMap.h
# End Source File
# Begin Source File

SOURCE=.\IntIntMap.h
# End Source File
# Begin Source File

SOURCE=.\IntLongMap.h
# End Source File
# Begin Source File

SOURCE=.\IntFloatMap.h
# End Source File
# Begin Source File

SOURCE=.\IntDoubleMap.h
# End Source File
# Begin Source File

SOURCE=.\IntSeq1MapOld.h
# End Source File
# Begin Source File

SOURCE=.\IntSeq2MapOld.h
# End Source File
# Begin Source File

SOURCE=.\IntSeq3MapOld.h
# End Source File
# Begin Source File

SOURCE=.\IntSeq4MapOld.h
# End Source File
# Begin Source File

SOURCE=.\IntE1MapOld.h
# End Source File
# Begin Source File

SOURCE=.\IntD1MapOld.h
# End Source File
# Begin Source File

SOURCE=.\IntS1MapOld.h
# End Source File
# Begin Source File

SOURCE=.\IntC1MapOld.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\TestOld.ice

!IF  "$(CFG)" == "transformP - Win32 Release"

USERDEP__TESTO="..\..\..\bin\slice2cpp.exe"	"..\..\..\bin\slice2xsd.exe"	"..\..\..\bin\slice2freeze.exe"
# Begin Custom Build
InputPath=.\TestOld.ice

BuildCmds= \
	set PATH=%PATH%;..\..\..\lib \
	..\..\..\bin\slice2cpp.exe TestOld.ice \
	..\..\..\bin\slice2xsd.exe TestOld.ice \
	..\..\..\bin\slice2freeze.exe --dict IntByteMap,int,byte IntByteMap \
	..\..\..\bin\slice2freeze.exe --dict IntShortMap,int,short IntShortMap \
	..\..\..\bin\slice2freeze.exe --dict IntIntMap,int,int IntIntMap \
	..\..\..\bin\slice2freeze.exe --dict IntLongMap,int,long IntLongMap \
	..\..\..\bin\slice2freeze.exe --dict IntFloatMap,int,float IntFloatMap \
	..\..\..\bin\slice2freeze.exe --dict IntDoubleMap,int,double IntDoubleMap \
	..\..\..\bin\slice2freeze.exe --dict IntSeq1Map,int,Test::Seq1 IntSeq1MapOld TestOld.ice \
	..\..\..\bin\slice2freeze.exe --dict IntSeq2Map,int,Test::Seq2 IntSeq2MapOld TestOld.ice \
	..\..\..\bin\slice2freeze.exe --dict IntSeq3Map,int,Test::Seq3 IntSeq3MapOld TestOld.ice \
	..\..\..\bin\slice2freeze.exe --dict IntSeq4Map,int,Test::Seq4 IntSeq4MapOld TestOld.ice \
	..\..\..\bin\slice2freeze.exe --dict IntE1Map,int,Test::E1 IntE1MapOld TestOld.ice \
	..\..\..\bin\slice2freeze.exe --dict IntD1Map,int,Test::D1 IntD1MapOld TestOld.ice \
	..\..\..\bin\slice2freeze.exe --dict IntS1Map,int,Test::S1 IntS1MapOld TestOld.ice \
	..\..\..\bin\slice2freeze.exe --dict IntC1Map,int,Test::C1 IntC1MapOld TestOld.ice \
	

"TestOld.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestOld.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestOld.xsd" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntByteMap.cpp" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntByteMap.h" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntShortMap.cpp" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntShortMap.h" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntIntMap.cpp" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntIntMap.h" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntLongMap.cpp" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntLongMap.h" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntFloatMap.cpp" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntFloatMap.h" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntDoubleMap.cpp" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntDoubleMap.h" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq1MapOld.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq1MapOld.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq2MapOld.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq2MapOld.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq3MapOld.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq3MapOld.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq4MapOld.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq4MapOld.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntE1MapOld.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntE1MapOld.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntD1MapOld.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntD1MapOld.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntS1MapOld.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntS1MapOld.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntC1MapOld.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntC1MapOld.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "transformP - Win32 Debug"

USERDEP__TESTO="..\..\..\bin\slice2cpp.exe"	"..\..\..\bin\slice2xsd.exe"	"..\..\..\bin\slice2freeze.exe"
# Begin Custom Build
InputPath=.\TestOld.ice

BuildCmds= \
	set PATH=%PATH%;..\..\..\lib \
	..\..\..\bin\slice2cpp.exe TestOld.ice \
	..\..\..\bin\slice2xsd.exe TestOld.ice \
	..\..\..\bin\slice2freeze.exe --dict IntByteMap,int,byte IntByteMap \
	..\..\..\bin\slice2freeze.exe --dict IntShortMap,int,short IntShortMap \
	..\..\..\bin\slice2freeze.exe --dict IntIntMap,int,int IntIntMap \
	..\..\..\bin\slice2freeze.exe --dict IntLongMap,int,long IntLongMap \
	..\..\..\bin\slice2freeze.exe --dict IntFloatMap,int,float IntFloatMap \
	..\..\..\bin\slice2freeze.exe --dict IntDoubleMap,int,double IntDoubleMap \
	..\..\..\bin\slice2freeze.exe --dict IntSeq1Map,int,Test::Seq1 IntSeq1MapOld TestOld.ice \
	..\..\..\bin\slice2freeze.exe --dict IntSeq2Map,int,Test::Seq2 IntSeq2MapOld TestOld.ice \
	..\..\..\bin\slice2freeze.exe --dict IntSeq3Map,int,Test::Seq3 IntSeq3MapOld TestOld.ice \
	..\..\..\bin\slice2freeze.exe --dict IntSeq4Map,int,Test::Seq4 IntSeq4MapOld TestOld.ice \
	..\..\..\bin\slice2freeze.exe --dict IntE1Map,int,Test::E1 IntE1MapOld TestOld.ice \
	..\..\..\bin\slice2freeze.exe --dict IntD1Map,int,Test::D1 IntD1MapOld TestOld.ice \
	..\..\..\bin\slice2freeze.exe --dict IntS1Map,int,Test::S1 IntS1MapOld TestOld.ice \
	..\..\..\bin\slice2freeze.exe --dict IntC1Map,int,Test::C1 IntC1MapOld TestOld.ice \
	

"TestOld.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestOld.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestOld.xsd" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntByteMap.cpp" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntByteMap.h" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntShortMap.cpp" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntShortMap.h" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntIntMap.cpp" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntIntMap.h" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntLongMap.cpp" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntLongMap.h" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntFloatMap.cpp" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntFloatMap.h" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntDoubleMap.cpp" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntDoubleMap.h" : "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq1MapOld.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq1MapOld.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq2MapOld.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq2MapOld.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq3MapOld.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq3MapOld.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq4MapOld.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntSeq4MapOld.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntE1MapOld.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntE1MapOld.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntD1MapOld.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntD1MapOld.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntS1MapOld.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntS1MapOld.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntC1MapOld.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IntC1MapOld.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TestNew.ice

!IF  "$(CFG)" == "transformP - Win32 Release"

USERDEP__TESTN="..\..\..\bin\slice2xsd.exe"	
# Begin Custom Build
InputPath=.\TestNew.ice

BuildCmds= \
	set PATH=%PATH%;..\..\..\lib \
	..\..\..\bin\slice2xsd.exe TestNew.ice \
	

"TestNew.xsd" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "transformP - Win32 Debug"

USERDEP__TESTN="..\..\..\bin\slice2xsd.exe"	
# Begin Custom Build
InputPath=.\TestNew.ice

BuildCmds= \
	set PATH=%PATH%;..\..\..\lib \
	..\..\..\bin\slice2xsd.exe TestNew.ice \
	

"TestNew.xsd" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
