# Microsoft Developer Studio Project File - Name="Freeze" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Freeze - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Freeze.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Freeze.mak" CFG="Freeze - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Freeze - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Freeze - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Freeze - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I ".." /I "../../include" /D "NDEBUG" /D "_USRDLL" /D "FREEZE_API_EXPORTS" /D "WIN32" /D "_CONSOLE" /D "_UNICODE" /YX /FD /c
# SUBTRACT CPP /Z<none> /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ws2_32.lib jtc.lib libdb33.lib /nologo /dll /machine:I386 /out:"Release/freeze001.dll" /libpath:"../../../lib"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Release\freeze001.* ..\..\lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Freeze - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "_DEBUG" /D "_USRDLL" /D "FREEZE_API_EXPORTS" /D "WIN32" /D "_CONSOLE" /D "_UNICODE" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib jtcd.lib libdb33d.lib /nologo /dll /debug /machine:I386 /out:"Debug/freeze001d.dll" /pdbtype:sept /libpath:"../../../lib"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Debug\freeze001d.* ..\..\lib
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Freeze - Win32 Release"
# Name "Freeze - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Application.cpp
# End Source File
# Begin Source File

SOURCE=.\DB.cpp
# End Source File
# Begin Source File

SOURCE=.\DBException.cpp
# End Source File
# Begin Source File

SOURCE=.\DBI.cpp
# End Source File
# Begin Source File

SOURCE=.\Evictor.cpp
# End Source File
# Begin Source File

SOURCE=.\EvictorI.cpp
# End Source File
# Begin Source File

SOURCE=.\IdentityObjectDict.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\Freeze\Application.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Freeze\DB.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Freeze\DBException.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Freeze\DBF.h
# End Source File
# Begin Source File

SOURCE=.\DBI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Freeze\Evictor.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Freeze\EvictorF.h
# End Source File
# Begin Source File

SOURCE=.\EvictorI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Freeze\Freeze.h
# End Source File
# Begin Source File

SOURCE=.\IdentityObjectDict.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Freeze\Initialize.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Freeze\Map.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\slice\Freeze\DB.ice

!IF  "$(CFG)" == "Freeze - Win32 Release"

USERDEP__DB_IC="..\..\bin\slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Freeze\DB.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/DB.ice \
	move DB.h ..\..\include\Freeze \
	

"..\..\include\Freeze\DB.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"DB.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Freeze - Win32 Debug"

USERDEP__DB_IC="..\..\bin\slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Freeze\DB.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/DB.ice \
	move DB.h ..\..\include\Freeze \
	

"..\..\include\Freeze\DB.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"DB.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Freeze\DBException.ice

!IF  "$(CFG)" == "Freeze - Win32 Release"

USERDEP__DBEXC="..\..\bin\slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Freeze\DBException.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/DBException.ice \
	move DBException.h ..\..\include\Freeze \
	

"..\..\include\Freeze\DBException.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"DBException.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Freeze - Win32 Debug"

USERDEP__DBEXC="..\..\bin\slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Freeze\DBException.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/DBException.ice \
	move DBException.h ..\..\include\Freeze \
	

"..\..\include\Freeze\DBException.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"DBException.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Freeze\DBF.ice

!IF  "$(CFG)" == "Freeze - Win32 Release"

USERDEP__DBF_I="..\..\bin\slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Freeze\DBF.ice

"..\..\include\Freeze\DBF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/DBF.ice 
	move DBF.h ..\..\include\Freeze 
	del DBF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Freeze - Win32 Debug"

USERDEP__DBF_I="..\..\bin\slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Freeze\DBF.ice

"..\..\include\Freeze\DBF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/DBF.ice 
	move DBF.h ..\..\include\Freeze 
	del DBF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Freeze\Evictor.ice

!IF  "$(CFG)" == "Freeze - Win32 Release"

USERDEP__EVICT="..\..\bin\slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Freeze\Evictor.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/Evictor.ice \
	move Evictor.h ..\..\include\Freeze \
	

"..\..\include\Freeze\Evictor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Evictor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Freeze - Win32 Debug"

USERDEP__EVICT="..\..\bin\slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Freeze\Evictor.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/Evictor.ice \
	move Evictor.h ..\..\include\Freeze \
	

"..\..\include\Freeze\Evictor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Evictor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Freeze\EvictorF.ice

!IF  "$(CFG)" == "Freeze - Win32 Release"

USERDEP__EVICTO="..\..\bin\slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Freeze\EvictorF.ice

"..\..\include\Freeze\EvictorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/EvictorF.ice 
	move EvictorF.h ..\..\include\Freeze 
	del EvictorF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Freeze - Win32 Debug"

USERDEP__EVICTO="..\..\bin\slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Freeze\EvictorF.ice

"..\..\include\Freeze\EvictorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/EvictorF.ice 
	move EvictorF.h ..\..\include\Freeze 
	del EvictorF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Identity.ice

!IF  "$(CFG)" == "Freeze - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\Ice\Identity.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2freeze.exe --include-dir Freeze -I../../slice --dict Freeze::IdentityObjectDict,Ice::Identity,Object IdentityObjectDict ../../slice/Ice/Identity.ice \
	

"IdentityObjectDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IdentityObjectDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Freeze - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\Ice\Identity.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2freeze.exe --include-dir Freeze -I../../slice --dict Freeze::IdentityObjectDict,Ice::Identity,Object IdentityObjectDict ../../slice/Ice/Identity.ice \
	

"IdentityObjectDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IdentityObjectDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
