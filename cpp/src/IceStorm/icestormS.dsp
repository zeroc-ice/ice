# Microsoft Developer Studio Project File - Name="IceStormS" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=IceStormS - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "IceStormS.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "IceStormS.mak" CFG="IceStormS - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "IceStormS - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "IceStormS - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "IceStormS - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I ".." /I "../../include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_UNICODE" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ws2_32.lib jtc.lib /nologo /subsystem:console /machine:I386 /out:"../../bin/icestorm.exe" /libpath:"../../../lib"

!ELSEIF  "$(CFG)" == "IceStormS - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_UNICODE" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib jtcd.lib /nologo /subsystem:console /debug /machine:I386 /out:"../../bin/icestorm.exe" /pdbtype:sept /libpath:"../../../lib"

!ENDIF 

# Begin Target

# Name "IceStormS - Win32 Release"
# Name "IceStormS - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Flusher.cpp
# End Source File
# Begin Source File

SOURCE=.\IceStormInternal.cpp
# End Source File
# Begin Source File

SOURCE=.\IdentityLinkDict.cpp
# End Source File
# Begin Source File

SOURCE=.\LinkInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\LinkSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=.\OnewayBatchSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=.\OnewaySubscriber.cpp
# End Source File
# Begin Source File

SOURCE=.\Server.cpp
# End Source File
# Begin Source File

SOURCE=.\StringBoolDict.cpp
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
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
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

SOURCE=.\IceStormInternal.h
# End Source File
# Begin Source File

SOURCE=.\IdentityLinkDict.h
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

SOURCE=.\StringBoolDict.h
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

SOURCE=.\dummy.ice

!IF  "$(CFG)" == "IceStormS - Win32 Release"

# Begin Custom Build
InputPath=.\dummy.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2freeze.exe --include-dir IceStorm  --dict IceStorm::StringBoolDict,string,bool StringBoolDict \
	

"StringBoolDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"StringBoolDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IceStormS - Win32 Debug"

# Begin Custom Build
InputPath=.\dummy.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2freeze.exe --include-dir IceStorm  --dict IceStorm::StringBoolDict,string,bool StringBoolDict \
	

"StringBoolDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"StringBoolDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dummy2.ice

!IF  "$(CFG)" == "IceStormS - Win32 Release"

# Begin Custom Build
InputPath=.\dummy2.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2freeze.exe --include-dir IceStorm  -I.. -I..\..\slice --dict IceStorm::IdentityLinkDict,Ice::Identity,IceStorm::LinkInfo IdentityLinkDict LinkInfo.ice \
	

"IdentityLinkDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IdentityLinkDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IceStormS - Win32 Debug"

# Begin Custom Build
InputPath=.\dummy2.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2freeze.exe --include-dir IceStorm  -I.. -I..\..\slice --dict IceStorm::IdentityLinkDict,Ice::Identity,IceStorm::LinkInfo IdentityLinkDict LinkInfo.ice \
	

"IdentityLinkDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IdentityLinkDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\IceStormInternal.ice

!IF  "$(CFG)" == "IceStormS - Win32 Release"

# Begin Custom Build
InputPath=.\IceStormInternal.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --include-dir IceStorm -I../../slice IceStormInternal.ice \
	

"IceStormInternal.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IceStormInternal.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IceStormS - Win32 Debug"

# Begin Custom Build
InputPath=.\IceStormInternal.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --include-dir IceStorm -I../../slice IceStormInternal.ice \
	

"IceStormInternal.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IceStormInternal.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LinkInfo.ice

!IF  "$(CFG)" == "IceStormS - Win32 Release"

# Begin Custom Build
InputPath=.\LinkInfo.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --include-dir IceStorm -I../../slice -I.. LinkInfo.ice \
	

"LinkInfo.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LinkInfo.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IceStormS - Win32 Debug"

# Begin Custom Build
InputPath=.\LinkInfo.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --include-dir IceStorm -I../../slice -I.. LinkInfo.ice \
	

"LinkInfo.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LinkInfo.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
