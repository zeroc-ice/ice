# Microsoft Developer Studio Project File - Name="icegridregistry" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=icegridregistry - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "icegridregistry.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "icegridregistry.mak" CFG="icegridregistry - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "icegridregistry - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "icegridregistry - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "icegridregistry - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /Zi /O2 /I "." /I ".." /I "../../include" /I "dummyinclude" /D "_CONSOLE" /D "NDEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /Zm200 /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 setargv.obj /nologo /subsystem:console /incremental:yes /machine:I386 /libpath:"../../../lib" /FIXED:no
# SUBTRACT LINK32 /debug /nodefaultlib
# Begin Special Build Tool
OutDir=.\Release
TargetName=icegridregistry
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\$(TargetName).exe ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "icegridregistry - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I "." /I ".." /I "../../include" /I "dummyinclude" /D "_CONSOLE" /D "_DEBUG" /D "WIN32_LEAN_AND_MEAN" /FD /Zm200 /GZ /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 setargv.obj /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"../../../lib" /FIXED:no
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
OutDir=.\Debug
TargetName=icegridregistry
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\$(TargetName).exe ..\..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "icegridregistry - Win32 Release"
# Name "icegridregistry - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AdapterCache.cpp
# End Source File
# Begin Source File

SOURCE=.\AdminI.cpp
# End Source File
# Begin Source File

SOURCE=.\Database.cpp
# End Source File
# Begin Source File

SOURCE=.\DescriptorBuilder.cpp
# End Source File
# Begin Source File

SOURCE=.\DescriptorHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\DescriptorParser.cpp
# End Source File
# Begin Source File

SOURCE=.\icegridregistry.cpp
# End Source File
# Begin Source File

SOURCE=.\IdentityObjectInfoDict.cpp
# End Source File
# Begin Source File

SOURCE=.\Internal.cpp
# End Source File
# Begin Source File

SOURCE=.\LocatorI.cpp
# End Source File
# Begin Source File

SOURCE=.\LocatorRegistryI.cpp
# End Source File
# Begin Source File

SOURCE=.\NodeCache.cpp
# End Source File
# Begin Source File

SOURCE=.\NodeSessionI.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectCache.cpp
# End Source File
# Begin Source File

SOURCE=.\ObserverSessionI.cpp
# End Source File
# Begin Source File

SOURCE=.\QueryI.cpp
# End Source File
# Begin Source File

SOURCE=.\ReapThread.cpp
# End Source File
# Begin Source File

SOURCE=.\RegistryI.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerCache.cpp
# End Source File
# Begin Source File

SOURCE=.\SessionManagerI.cpp
# End Source File
# Begin Source File

SOURCE=.\StringAdapterInfoDict.cpp
# End Source File
# Begin Source File

SOURCE=.\StringApplicationDescriptorDict.cpp
# End Source File
# Begin Source File

SOURCE=.\Topics.cpp
# End Source File
# Begin Source File

SOURCE=.\TraceLevels.cpp
# End Source File
# Begin Source File

SOURCE=.\Util.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AdapterFactory.h
# End Source File
# Begin Source File

SOURCE=.\AdapterI.h
# End Source File
# Begin Source File

SOURCE=.\AdapterRegistryI.h
# End Source File
# Begin Source File

SOURCE=.\AdminI.h
# End Source File
# Begin Source File

SOURCE=.\ApplicationBuilder.h
# End Source File
# Begin Source File

SOURCE=.\ComponentBuilder.h
# End Source File
# Begin Source File

SOURCE=.\IdentityObjectInfoDict.h
# End Source File
# Begin Source File

SOURCE=.\Internal.h
# End Source File
# Begin Source File

SOURCE=.\LocatorI.h
# End Source File
# Begin Source File

SOURCE=.\LocatorRegistryI.h
# End Source File
# Begin Source File

SOURCE=.\NodeRegistryI.h
# End Source File
# Begin Source File

SOURCE=.\ObjectRegistryI.h
# End Source File
# Begin Source File

SOURCE=.\QueryI.h
# End Source File
# Begin Source File

SOURCE=.\Registry.h
# End Source File
# Begin Source File

SOURCE=.\ServerRegistryI.h
# End Source File
# Begin Source File

SOURCE=.\StringApplicationDescriptorDict.h
# End Source File
# Begin Source File

SOURCE=.\StringProxyDict.h
# End Source File
# Begin Source File

SOURCE=.\TraceLevels.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\dummy1.ice

!IF  "$(CFG)" == "icegridregistry - Win32 Release"

USERDEP__DUMMY="..\..\bin\slice2freeze.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=.\dummy1.ice

BuildCmds= \
	..\..\bin\slice2freeze.exe --ice --include-dir IceGrid -I../../slice -I.. --dict IceGrid::IdentityObjectInfoDict,Ice::Identity,IceGrid::ObjectInfo --dict-index IceGrid::IdentityObjectInfoDict,type IdentityObjectInfoDict ../../slice/Ice/Identity.ice ../../slice/IceGrid/Admin.ice

"IdentityObjectInfoDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IdentityObjectInfoDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icegridregistry - Win32 Debug"

USERDEP__DUMMY="..\..\bin\slice2freeze.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=.\dummy1.ice

BuildCmds= \
	..\..\bin\slice2freeze.exe --ice --include-dir IceGrid -I../../slice -I.. --dict IceGrid::IdentityObjectInfoDict,Ice::Identity,IceGrid::ObjectInfo --dict-index IceGrid::IdentityObjectInfoDict,type IdentityObjectInfoDict ../../slice/Ice/Identity.ice ../../slice/IceGrid/Admin.ice

"IdentityObjectInfoDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"IdentityObjectInfoDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dummy2.ice

!IF  "$(CFG)" == "icegridregistry - Win32 Release"

USERDEP__DUMMY2="..\..\bin\slice2freeze.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=.\dummy2.ice

BuildCmds= \
	..\..\bin\slice2freeze.exe --ice --include-dir IceGrid -I../../slice --dict "IceGrid::StringAdapterInfoDict,string,IceGrid::AdapterInfo" StringAdapterInfoDict --dict-index "IceGrid::StringAdapterInfoDict,replicaGroupId" ../../slice/IceGrid/Admin.ice

"StringAdapterInfoDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"StringAdapterInfoDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icegridregistry - Win32 Debug"

USERDEP__DUMMY2="..\..\bin\slice2freeze.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=.\dummy2.ice

BuildCmds= \
	..\..\bin\slice2freeze.exe --ice --include-dir IceGrid -I../../slice --dict "IceGrid::StringAdapterInfoDict,string,IceGrid::AdapterInfo" StringAdapterInfoDict --dict-index "IceGrid::StringAdapterInfoDict,replicaGroupId" ../../slice/IceGrid/Admin.ice

"StringAdapterInfoDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"StringAdapterInfoDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dummy3.ice

!IF  "$(CFG)" == "icegridregistry - Win32 Release"

USERDEP__DUMMY3="..\..\bin\slice2freeze.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=.\dummy3.ice

BuildCmds= \
	..\..\bin\slice2freeze.exe --ice --include-dir IceGrid -I../../slice -I.. --dict IceGrid::StringApplicationDescriptorDict,string,IceGrid::ApplicationDescriptor StringApplicationDescriptorDict ../../slice/IceGrid/Admin.ice

"StringApplicationDescriptorDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"StringApplicationDescriptorDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icegridregistry - Win32 Debug"

USERDEP__DUMMY3="..\..\bin\slice2freeze.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=.\dummy3.ice

BuildCmds= \
	..\..\bin\slice2freeze.exe --ice --include-dir IceGrid -I../../slice --dict IceGrid::StringApplicationDescriptorDict,string,IceGrid::ApplicationDescriptor StringApplicationDescriptorDict ../../slice/IceGrid/Admin.ice

"StringApplicationDescriptorDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"StringApplicationDescriptorDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Internal.ice

!IF  "$(CFG)" == "icegridregistry - Win32 Release"

USERDEP__INTER="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=.\Internal.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --include-dir IceGrid -I../../slice -I.. Internal.ice

".\Internal.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Internal.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icegridregistry - Win32 Debug"

USERDEP__INTER="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=.\Internal.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --include-dir IceGrid -I../../slice -I.. Internal.ice

".\Internal.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Internal.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
