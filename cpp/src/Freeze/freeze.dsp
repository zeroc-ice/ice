# Microsoft Developer Studio Project File - Name="freeze" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=freeze - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "freeze.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "freeze.mak" CFG="freeze - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "freeze - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "freeze - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "freeze - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I ".." /I "../../include" /D "_USRDLL" /D "FREEZE_API_EXPORTS" /D "_CONSOLE" /D "NDEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /c
# SUBTRACT CPP /Z<none> /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 libdb43.lib /nologo /dll /pdb:none /machine:I386 /out:"Release/freeze31.dll" /implib:"Release/freeze.lib" /FIXED:no
# SUBTRACT LINK32 /debug
# Begin Special Build Tool
OutDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\freeze.lib ..\..\lib	copy $(OutDir)\freeze31.dll ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "freeze - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "_USRDLL" /D "FREEZE_API_EXPORTS" /D "_CONSOLE" /D "_DEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /GZ /c
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
# ADD LINK32 libdb43d.lib /nologo /dll /debug /machine:I386 /out:"Debug/freeze31d.dll" /implib:"Debug/freezed.lib" /FIXED:no
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
OutDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\freezed.lib ..\..\lib	copy $(OutDir)\freeze31d.pdb ..\..\bin	copy $(OutDir)\freeze31d.dll ..\..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "freeze - Win32 Release"
# Name "freeze - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Catalog.cpp
# End Source File
# Begin Source File

SOURCE=.\CatalogData.cpp
# End Source File
# Begin Source File

SOURCE=.\Connection.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnectionF.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnectionI.cpp
# End Source File
# Begin Source File

SOURCE=.\DB.cpp
# End Source File
# Begin Source File

SOURCE=.\Evictor.cpp
# End Source File
# Begin Source File

SOURCE=.\EvictorI.cpp
# End Source File
# Begin Source File

SOURCE=.\EvictorIteratorI.cpp
# End Source File
# Begin Source File

SOURCE=.\EvictorStorage.cpp
# End Source File
# Begin Source File

SOURCE=.\Exception.cpp
# End Source File
# Begin Source File

SOURCE=.\Index.cpp
# End Source File
# Begin Source File

SOURCE=.\IndexI.cpp
# End Source File
# Begin Source File

SOURCE=.\MapI.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectStore.cpp
# End Source File
# Begin Source File

SOURCE=.\PingObject.cpp
# ADD CPP /I "."
# End Source File
# Begin Source File

SOURCE=.\SharedDb.cpp
# End Source File
# Begin Source File

SOURCE=.\SharedDbEnv.cpp
# End Source File
# Begin Source File

SOURCE=.\Transaction.cpp
# End Source File
# Begin Source File

SOURCE=.\TransactionHolder.cpp
# End Source File
# Begin Source File

SOURCE=.\TransactionI.cpp
# End Source File
# Begin Source File

SOURCE=.\Util.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\freeze\Application.h
# End Source File
# Begin Source File

SOURCE=..\..\include\freeze\Catalog.h
# End Source File
# Begin Source File

SOURCE=..\..\include\freeze\CatalogData.h
# End Source File
# Begin Source File

SOURCE=..\..\include\freeze\Connection.h
# End Source File
# Begin Source File

SOURCE=..\..\include\freeze\ConnectionF.h
# End Source File
# Begin Source File

SOURCE=.\ConnectionI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\freeze\DB.h
# End Source File
# Begin Source File

SOURCE=..\..\include\freeze\Evictor.h
# End Source File
# Begin Source File

SOURCE=..\..\include\freeze\EvictorF.h
# End Source File
# Begin Source File

SOURCE=.\EvictorI.h
# End Source File
# Begin Source File

SOURCE=.\EvictorIteratorI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\freeze\EvictorStorage.h
# End Source File
# Begin Source File

SOURCE=..\..\include\freeze\Exception.h
# End Source File
# Begin Source File

SOURCE=..\..\include\freeze\Freeze.h
# End Source File
# Begin Source File

SOURCE=..\..\include\freeze\Index.h
# End Source File
# Begin Source File

SOURCE=.\IndexI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\freeze\Initialize.h
# End Source File
# Begin Source File

SOURCE=..\..\include\freeze\Map.h
# End Source File
# Begin Source File

SOURCE=.\MapI.h
# End Source File
# Begin Source File

SOURCE=.\ObjectStore.h
# End Source File
# Begin Source File

SOURCE=.\PingObject.h
# End Source File
# Begin Source File

SOURCE=..\..\include\freeze\Transaction.h
# End Source File
# Begin Source File

SOURCE=..\..\include\freeze\TransactionHolder.h
# End Source File
# Begin Source File

SOURCE=.\TransactionI.h
# End Source File
# Begin Source File

SOURCE=.\Util.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\slice\freeze\CatalogData.ice

!IF  "$(CFG)" == "freeze - Win32 Release"

USERDEP__CATAL="..\..\bin\slice2cpp.exe"	"..\..\bin\slice2freeze.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\freeze\CatalogData.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/CatalogData.ice \
	move CatalogData.h ..\..\include\freeze \
	..\..\bin\slice2freeze.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice --dict Freeze::Catalog,string,Freeze::CatalogData Catalog ../../slice/Freeze/CatalogData.ice \
	move Catalog.h ..\..\include\freeze \
	

"..\..\include\freeze\CatalogData.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"CatalogData.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\..\include\freeze\Catalog.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Catalog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "freeze - Win32 Debug"

USERDEP__CATAL="..\..\bin\slice2cpp.exe"	"..\..\bin\slice2freeze.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\freeze\CatalogData.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/CatalogData.ice \
	move CatalogData.h ..\..\include\freeze \
	..\..\bin\slice2freeze.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice --dict Freeze::Catalog,string,Freeze::CatalogData Catalog ../../slice/Freeze/CatalogData.ice \
	move Catalog.h ..\..\include\freeze \
	

"..\..\include\freeze\CatalogData.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"CatalogData.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\..\include\freeze\Catalog.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Catalog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\freeze\Connection.ice

!IF  "$(CFG)" == "freeze - Win32 Release"

USERDEP__CONNE="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\freeze\Connection.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/Connection.ice \
	move Connection.h ..\..\include\freeze \
	

"..\..\include\freeze\Connection.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Connection.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "freeze - Win32 Debug"

USERDEP__CONNE="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\freeze\Connection.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/Connection.ice \
	move Connection.h ..\..\include\freeze \
	

"..\..\include\freeze\Connection.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Connection.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\freeze\ConnectionF.ice

!IF  "$(CFG)" == "freeze - Win32 Release"

USERDEP__CONNEC="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\freeze\ConnectionF.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/ConnectionF.ice \
	move ConnectionF.h ..\..\include\freeze \
	

"..\..\include\freeze\ConnectionF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ConnectionF.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "freeze - Win32 Debug"

USERDEP__CONNEC="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\freeze\ConnectionF.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/ConnectionF.ice \
	move ConnectionF.h ..\..\include\freeze \
	

"..\..\include\freeze\ConnectionF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ConnectionF.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\freeze\DB.ice

!IF  "$(CFG)" == "freeze - Win32 Release"

USERDEP__DB_IC="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\freeze\DB.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/DB.ice \
	move DB.h ..\..\include\freeze \
	

"..\..\include\freeze\DB.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"DB.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "freeze - Win32 Debug"

USERDEP__DB_IC="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\freeze\DB.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/DB.ice \
	move DB.h ..\..\include\freeze \
	

"..\..\include\freeze\DB.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"DB.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\freeze\Evictor.ice

!IF  "$(CFG)" == "freeze - Win32 Release"

USERDEP__EVICT="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\freeze\Evictor.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/Evictor.ice \
	move Evictor.h ..\..\include\freeze \
	

"..\..\include\freeze\Evictor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Evictor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "freeze - Win32 Debug"

USERDEP__EVICT="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\freeze\Evictor.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/Evictor.ice \
	move Evictor.h ..\..\include\freeze \
	

"..\..\include\freeze\Evictor.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Evictor.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\freeze\EvictorF.ice

!IF  "$(CFG)" == "freeze - Win32 Release"

USERDEP__EVICTO="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\freeze\EvictorF.ice

"..\..\include\freeze\EvictorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/EvictorF.ice 
	move EvictorF.h ..\..\include\freeze 
	del EvictorF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "freeze - Win32 Debug"

USERDEP__EVICTO="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\freeze\EvictorF.ice

"..\..\include\freeze\EvictorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/EvictorF.ice 
	move EvictorF.h ..\..\include\freeze 
	del EvictorF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\freeze\EvictorStorage.ice

!IF  "$(CFG)" == "freeze - Win32 Release"

USERDEP__EVICTOR="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\freeze\EvictorStorage.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/EvictorStorage.ice \
	move EvictorStorage.h ..\..\include\freeze \
	

"..\..\include\freeze\EvictorStorage.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"EvictorStorage.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "freeze - Win32 Debug"

USERDEP__EVICTOR="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\freeze\EvictorStorage.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/EvictorStorage.ice \
	move EvictorStorage.h ..\..\include\freeze \
	

"..\..\include\freeze\EvictorStorage.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"EvictorStorage.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\freeze\Exception.ice

!IF  "$(CFG)" == "freeze - Win32 Release"

USERDEP__EXCEP="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\freeze\Exception.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/Exception.ice \
	move Exception.h ..\..\include\freeze \
	

"..\..\include\freeze\Exception.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Exception.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "freeze - Win32 Debug"

USERDEP__EXCEP="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\freeze\Exception.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/Exception.ice \
	move Exception.h ..\..\include\freeze \
	

"..\..\include\freeze\Exception.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Exception.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PingObject.ice

!IF  "$(CFG)" == "freeze - Win32 Release"

# Begin Custom Build
InputPath=.\PingObject.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe PingObject.ice

"PingObject.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"PingObject.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "freeze - Win32 Debug"

# Begin Custom Build
InputPath=.\PingObject.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe PingObject.ice

"PingObject.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"PingObject.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\freeze\Transaction.ice

!IF  "$(CFG)" == "freeze - Win32 Release"

USERDEP__TRANS="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\freeze\Transaction.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/Transaction.ice \
	move Transaction.h ..\..\include\freeze \
	

"..\..\include\freeze\Transaction.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Transaction.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "freeze - Win32 Debug"

USERDEP__TRANS="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\freeze\Transaction.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --dll-export FREEZE_API --include-dir Freeze -I../../slice ../../slice/Freeze/Transaction.ice \
	move Transaction.h ..\..\include\freeze \
	

"..\..\include\freeze\Transaction.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Transaction.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
