# Microsoft Developer Studio Project File - Name="icestormS" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=icestormS - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iceStormS.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iceStormS.mak" CFG="icestormS - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "icestormS - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "icestormS - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "icestormS - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I ".." /I "../../include" /D "_USRDLL" /D "_CONSOLE" /D "NDEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /c
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
# ADD LINK32 /nologo /dll /incremental:yes /machine:I386 /out:"Release/icestormservice21.dll" /implib:"Release/icestormservice.lib" /FIXED:no
# SUBTRACT LINK32 /pdb:none /debug /nodefaultlib
# Begin Special Build Tool
OutDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\icestormservice.lib ..\..\lib	copy $(OutDir)\icestormservice21.dll ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "icestormS - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "_USRDLL" /D "ICE_STORM_SERVICE_API_EXPORTS" /D "_CONSOLE" /D "_DEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386
# ADD LINK32 /nologo /dll /debug /machine:I386 /out:"Debug/icestormservice21d.dll" /implib:"Debug/icestormserviced.lib" /FIXED:no
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
OutDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\icestormserviced.lib ..\..\lib	copy $(OutDir)\icestormservice21d.pdb ..\..\bin	copy $(OutDir)\icestormservice21d.dll ..\..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "icestormS - Win32 Release"
# Name "icestormS - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Flusher.cpp
# End Source File
# Begin Source File

SOURCE=.\IceStormInternal.cpp
# End Source File
# Begin Source File

SOURCE=.\LinkProxy.cpp
# End Source File
# Begin Source File

SOURCE=.\LinkRecord.cpp
# End Source File
# Begin Source File

SOURCE=.\LinkSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=.\OnewayBatchSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=.\OnewayProxy.cpp
# End Source File
# Begin Source File

SOURCE=.\OnewaySubscriber.cpp
# End Source File
# Begin Source File

SOURCE=.\PersistentTopicMap.cpp
# End Source File
# Begin Source File

SOURCE=.\QueuedProxy.cpp
# End Source File
# Begin Source File

SOURCE=.\Service.cpp
# End Source File
# Begin Source File

SOURCE=.\Subscriber.cpp
# End Source File
# Begin Source File

SOURCE=.\SubscriberFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\TopicI.cpp
# End Source File
# Begin Source File

SOURCE=.\TopicManagerI.cpp
# End Source File
# Begin Source File

SOURCE=.\TraceLevels.cpp
# End Source File
# Begin Source File

SOURCE=.\TwowayProxy.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Event.h
# End Source File
# Begin Source File

SOURCE=.\Flushable.h
# End Source File
# Begin Source File

SOURCE=.\Flusher.h
# End Source File
# Begin Source File

SOURCE=.\IceStormI.h
# End Source File
# Begin Source File

SOURCE=.\IceStormInternalI.h
# End Source File
# Begin Source File

SOURCE=.\LinkInfo.h
# End Source File
# Begin Source File

SOURCE=.\LinkSubscriber.h
# End Source File
# Begin Source File

SOURCE=.\OnewayBatchSubscriber.h
# End Source File
# Begin Source File

SOURCE=.\OnewaySubscriber.h
# End Source File
# Begin Source File

SOURCE=.\PersistentTopicMap.h
# End Source File
# Begin Source File

SOURCE=.\Subscriber.h
# End Source File
# Begin Source File

SOURCE=.\SubscriberFactory.h
# End Source File
# Begin Source File

SOURCE=.\TopicI.h
# End Source File
# Begin Source File

SOURCE=.\TopicManagerI.h
# End Source File
# Begin Source File

SOURCE=.\TraceLevels.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\IceStormInternal.ice

!IF  "$(CFG)" == "icestormS - Win32 Release"

USERDEP__ICEST="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=.\IceStormInternal.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --include-dir IceStorm -I../../slice IceStormInternal.ice

"IceStormInternal.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IceStormInternal.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icestormS - Win32 Debug"

USERDEP__ICEST="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=.\IceStormInternal.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --include-dir IceStorm -I../../slice IceStormInternal.ice

"IceStormInternal.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IceStormInternal.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LinkRecord.ice

!IF  "$(CFG)" == "icestormS - Win32 Release"

USERDEP__LINKR="..\..\bin\slice2freeze.exe"	"..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=.\LinkRecord.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --include-dir IceStorm -I../../slice -I.. LinkRecord.ice \
	..\..\bin\slice2freeze.exe --ice --include-dir IceStorm  -I../../slice -I.. --dict IceStorm::PersistentTopicMap,string,IceStorm::LinkRecordDict PersistentTopicMap ../IceStorm/LinkRecord.ice \
	

"LinkRecord.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LinkRecord.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"PersistentTopicMap.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"PersistentTopicMap.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icestormS - Win32 Debug"

USERDEP__LINKR="..\..\bin\slice2freeze.exe"	"..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=.\LinkRecord.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --include-dir IceStorm -I../../slice -I.. LinkRecord.ice \
	..\..\bin\slice2freeze.exe --ice --include-dir IceStorm  -I../../slice -I.. --dict IceStorm::PersistentTopicMap,string,IceStorm::LinkRecordDict PersistentTopicMap ../IceStorm/LinkRecord.ice \
	

"LinkRecord.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LinkRecord.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"PersistentTopicMap.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"PersistentTopicMap.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
