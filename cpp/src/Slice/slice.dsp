# Microsoft Developer Studio Project File - Name="slice" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=slice - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "slice.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "slice.mak" CFG="slice - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "slice - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "slice - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "slice - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PARSER_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I ".." /I "../../include" /I "dummyinclude" /D "_USRDLL" /D "SLICE_API_EXPORTS" /D "_CONSOLE" /D "NDEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /c
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
# ADD LINK32 /nologo /dll /incremental:yes /machine:I386 /out:"Release/slice21.dll" /implib:"Release/slice.lib"
# SUBTRACT LINK32 /pdb:none /debug /nodefaultlib
# Begin Special Build Tool
OutDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\slice.lib ..\..\lib	copy $(OutDir)\slice21.dll ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "slice - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PARSER_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /I "dummyinclude" /D "_USRDLL" /D "SLICE_API_EXPORTS" /D "_CONSOLE" /D "_DEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /GZ /c
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
# ADD LINK32 /nologo /dll /debug /machine:I386 /out:"Debug/slice21d.dll" /implib:"Debug/sliced.lib" /FIXED:no
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
OutDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\sliced.lib ..\..\lib	copy $(OutDir)\slice21d.pdb ..\..\bin	copy $(OutDir)\slice21d.dll ..\..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "slice - Win32 Release"
# Name "slice - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Checksum.cpp
# End Source File
# Begin Source File

SOURCE=.\CPlusPlusUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\CsUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\DotNetNames.cpp
# End Source File
# Begin Source File

SOURCE=.\Grammar.cpp
# End Source File
# Begin Source File

SOURCE=.\JavaUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\Parser.cpp
# End Source File
# Begin Source File

SOURCE=.\Preprocessor.cpp
# End Source File
# Begin Source File

SOURCE=.\PythonUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\Scanner.cpp
# End Source File
# Begin Source File

SOURCE=.\VbUtil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\incluce\slice\Checksum.h
# End Source File
# Begin Source File

SOURCE=..\..\incluce\slice\CPlusPlusUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\incluce\slice\CsUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\slice\DotNetNames.h
# End Source File
# Begin Source File

SOURCE=.\Grammar.h
# End Source File
# Begin Source File

SOURCE=.\GrammarUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\incluce\slice\OutputUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\slice\Parser.h
# End Source File
# Begin Source File

SOURCE=..\..\include\slice\Preprocessor.h
# End Source File
# Begin Source File

SOURCE=..\..\include\slice\PythonUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\slice\VbUtil.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Grammar.y

!IF  "$(CFG)" == "slice - Win32 Release"

# Begin Custom Build
InputPath=.\Grammar.y

BuildCmds= \
	bison -dvt --name-prefix "slice_" Grammar.y \
	move Grammar.tab.c Grammar.cpp \
	move Grammar.tab.h Grammar.h \
	

"Grammar.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Grammar.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "slice - Win32 Debug"

# Begin Custom Build
InputPath=.\Grammar.y

BuildCmds= \
	bison -dvt --name-prefix "slice_" Grammar.y \
	move Grammar.tab.c Grammar.cpp \
	move Grammar.tab.h Grammar.h \
	

"Grammar.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Grammar.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Scanner.l

!IF  "$(CFG)" == "slice - Win32 Release"

# Begin Custom Build
InputPath=.\Scanner.l

"Scanner.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex Scanner.l 
	echo #include "IceUtil/Config.h" > Scanner.cpp 
	type lex.yy.c >> Scanner.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "slice - Win32 Debug"

# Begin Custom Build
InputPath=.\Scanner.l

"Scanner.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex Scanner.l 
	echo #include "IceUtil/Config.h" > Scanner.cpp 
	type lex.yy.c >> Scanner.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
