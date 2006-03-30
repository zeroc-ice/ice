# Microsoft Developer Studio Project File - Name="icec" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=icec - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "icec.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "icec.mak" CFG="icec - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "icec - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "icec - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "icec - Win32 Release Static" (based on "Win32 (x86) Static Library")
!MESSAGE "icec - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "icec - Win32 Release"

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
SLICE2CPPEFLAGS=-DICEE
CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBRARY_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O1 /I ".." /I "../../include" /D "NDEBUG" /D "_USRDLL" /D "ICE_API_EXPORTS" /D "WIN32_LEAN_AND_MEAN" /D FD_SETSIZE=1024 /D "_CONSOLE" /D "ICEE_PURE_CLIENT" /FD /c
# SUBTRACT CPP /Fr /YX
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ws2_32.lib rpcrt4.lib /nologo /dll /incremental:yes /debug /machine:I386 /out:"Release/iceec11.dll" /implib:"Release/iceec.lib"
# SUBTRACT LINK32 /pdb:none /nodefaultlib
# Begin Special Build Tool
OutDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\iceec.lib ..\..\lib\	copy $(OutDir)\iceec11.dll ..\..\bin	copy $(OutDir)\iceec11.pdb ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "icec - Win32 Debug"

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
SLICE2CPPEFLAGS=-DICEE
CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBRARY_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "_DEBUG" /D "_USRDLL" /D "ICE_API_EXPORTS" /D "WIN32_LEAN_AND_MEAN" /D FD_SETSIZE=1024 /D "_CONSOLE" /D "ICEE_PURE_CLIENT" /FD /GZ /c
# SUBTRACT CPP /Fr /YX
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386
# ADD LINK32 ws2_32.lib rpcrt4.lib /nologo /dll /debug /machine:I386 /out:"Debug/iceec11d.dll" /implib:"Debug/iceecd.lib"
# SUBTRACT LINK32 /profile /pdb:none /incremental:no /nodefaultlib
# Begin Special Build Tool
OutDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\iceecd.lib ..\..\lib\	copy $(OutDir)\iceec11d.pdb ..\..\bin	copy $(OutDir)\iceec11d.dll ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "icec - Win32 Release Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseStatic"
# PROP BASE Intermediate_Dir "ReleaseStatic"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseStatic"
# PROP Intermediate_Dir "ReleaseStatic"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
SLICE2CPPEFLAGS=-DICEE
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O1 /I ".." /I "../../include" /D "NDEBUG" /D "ICEE_STATIC_LIBS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /D FD_SETSIZE=1024 /D "_CONSOLE" /D "ICEE_PURE_CLIENT" /FD /c
# SUBTRACT CPP /Fr /YX
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD LIB32 /nologo /out:"ReleaseStatic\iceec.lib"
# Begin Special Build Tool
OutDir=.\ReleaseStatic
SOURCE="$(InputPath)"
PostBuild_Cmds=copy                                               $(OutDir)\iceec.lib                                               ..\..\lib\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "icec - Win32 Debug Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugStatic"
# PROP BASE Intermediate_Dir "DebugStatic"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugStatic"
# PROP Intermediate_Dir "DebugStatic"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
SLICE2CPPEFLAGS=-DICEE
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "_DEBUG" /D "ICEE_STATIC_LIBS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /D FD_SETSIZE=1024 /D "_CONSOLE" /D "ICEE_PURE_CLIENT" /FD /GZ /c
# SUBTRACT CPP /Fr /YX
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"DebugStatic/iceec.bsc"
LIB32=link.exe -lib
# ADD LIB32 /nologo /out:"DebugStatic\iceecd.lib"
# Begin Special Build Tool
OutDir=.\DebugStatic
SOURCE="$(InputPath)"
PostBuild_Cmds=copy                                              $(OutDir)\iceecd.lib                                              ..\..\lib\ 
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "icec - Win32 Release"
# Name "icec - Win32 Debug"
# Name "icec - Win32 Release Static"
# Name "icec - Win32 Debug Static"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\IceE\BasicStream.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\Buffer.cpp
# End Source File
# Begin Source File

SOURCE=.\BuiltinSequences.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\Communicator.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\Cond.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\Connection.cpp
# End Source File
# Begin Source File

SOURCE=..\TcpTransport\Connector.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\ConvertUTF.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\DefaultsAndOverrides.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\Endpoint.cpp
# End Source File
# Begin Source File

SOURCE=..\TcpTransport\EndpointFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\ExceptionBase.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\FactoryTable.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\FactoryTableDef.cpp
# End Source File
# Begin Source File

SOURCE=.\Identity.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\IdentityUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\Initialize.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\Instance.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\LocalException.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\LocalObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Locator.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\LocatorInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Logger.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\LoggerI.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\LoggerUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\Network.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\OperationMode.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\Outgoing.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\OutgoingConnectionFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\Properties.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\Protocol.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\Proxy.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\ProxyFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\RecMutex.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\Reference.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\ReferenceFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\Router.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\RouterInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\RoutingTable.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\RWRecMutex.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\SafeStdio.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\Shared.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\StaticMutex.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\StringUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\TcpTransport\TcpEndpoint.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\Thread.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\ThreadException.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\Time.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\TraceLevels.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\TraceUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\TcpTransport\Transceiver.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\Unicode.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\UnknownEndpoint.cpp
# End Source File
# Begin Source File

SOURCE=..\IceE\UUID.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\IceEUtil\AbstractMutex.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Acceptor.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\AcceptorF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceEUtil\Algorithm.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\BasicStream.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Buffer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Communicator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\CommunicatorF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Cond.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Config.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Connection.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\ConnectionF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Connector.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\ConnectorF.h
# End Source File
# Begin Source File

SOURCE=..\IceE\ConvertUTF.h
# End Source File
# Begin Source File

SOURCE=..\IceE\DefaultsAndOverrides.h
# End Source File
# Begin Source File

SOURCE=..\IceE\DefaultsAndOverridesF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Endpoint.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\EndpointF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\EndpointFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\EndpointFactoryF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Exception.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Exception.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\FactoryTable.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\FactoryTableDef.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Functional.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Functional.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Handle.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Handle.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\IceE.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\IceEUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Identity.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\IdentityUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Incoming.h
# End Source File
# Begin Source File

SOURCE=..\IceE\IncomingConnectionFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Initialize.h
# End Source File
# Begin Source File

SOURCE=..\IceE\Instance.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\InstanceF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceEUtil\Iterator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\LocalException.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\LocalObject.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\LocalObjectF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Locator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\LocatorF.h
# End Source File
# Begin Source File

SOURCE=..\IceE\LocatorInfo.h
# End Source File
# Begin Source File

SOURCE=..\IceE\LocatorInfoF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Lock.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Logger.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\LoggerF.h
# End Source File
# Begin Source File

SOURCE=..\IceE\LoggerI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\LoggerUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Monitor.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Mutex.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Native.h
# End Source File
# Begin Source File

SOURCE=..\IceE\Network.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\OperationMode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Outgoing.h
# End Source File
# Begin Source File

SOURCE=..\IceE\OutgoingConnectionFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Properties.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\PropertiesF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Protocol.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Proxy.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\ProxyF.h
# End Source File
# Begin Source File

SOURCE=..\IceE\ProxyFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\ProxyFactoryF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\ProxyHandle.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\RecMutex.h
# End Source File
# Begin Source File

SOURCE=..\IceE\Reference.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\ReferenceF.h
# End Source File
# Begin Source File

SOURCE=..\IceE\ReferenceFactory.h
# End Source File
# Begin Source File

SOURCE=..\IceE\ReferenceFactoryF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\RoutingTable.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\RWRecMutex.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\SafeStdio.h
# End Source File
# Begin Source File

SOURCE=..\IceE\ServantManager.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\ServantManagerF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Shared.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\StaticMutex.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\StringUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Thread.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\ThreadException.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Time.h
# End Source File
# Begin Source File

SOURCE=..\IceE\TraceLevels.h
# End Source File
# Begin Source File

SOURCE=..\IceE\TraceLevelsF.h
# End Source File
# Begin Source File

SOURCE=..\IceE\TraceUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Transceiver.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\TransceiverF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Unicode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\UserExceptionFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\UserExceptionFactoryF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iceutil\UUID.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\slice\IceE\BuiltinSequences.ice

!IF  "$(CFG)" == "icec - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceE\BuiltinSequences.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/BuiltinSequences.ice \
	move BuiltinSequences.h. ..\..\include\icee \
	

"..\..\include\icee\BuiltinSequences.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"BuiltinSequences.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceE\BuiltinSequences.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/BuiltinSequences.ice \
	move BuiltinSequences.h. ..\..\include\icee \
	

"..\..\include\icee\BuiltinSequences.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"BuiltinSequences.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Release Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\BuiltinSequences.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/BuiltinSequences.ice \
	move BuiltinSequences.h. ..\..\include\icee \
	

"..\..\include\icee\BuiltinSequences.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"BuiltinSequences.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Debug Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\BuiltinSequences.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/BuiltinSequences.ice \
	move BuiltinSequences.h. ..\..\include\icee \
	

"..\..\include\icee\BuiltinSequences.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"BuiltinSequences.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceE\FacetMap.ice

!IF  "$(CFG)" == "icec - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceE\FacetMap.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice  ../../slice/IceE/FacetMap.ice \
	move FacetMap.h ..\..\include\icee \
	

"..\..\include\icee\FacetMap.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"FacetMap.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceE\FacetMap.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice  ../../slice/IceE/FacetMap.ice \
	move FacetMap.h ..\..\include\icee \
	

"..\..\include\icee\FacetMap.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"FacetMap.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Release Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\FacetMap.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice  ../../slice/IceE/FacetMap.ice \
	move FacetMap.h ..\..\include\icee \
	

"..\..\include\icee\FacetMap.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"FacetMap.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Debug Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\FacetMap.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice  ../../slice/IceE/FacetMap.ice \
	move FacetMap.h ..\..\include\icee \
	

"..\..\include\icee\FacetMap.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"FacetMap.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceE\Identity.ice

!IF  "$(CFG)" == "icec - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceE\Identity.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/Identity.ice \
	move Identity.h ..\..\include\icee \
	

"..\..\include\icee\Identity.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Identity.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceE\Identity.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/Identity.ice \
	move Identity.h ..\..\include\icee \
	

"..\..\include\icee\Identity.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Identity.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Release Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\Identity.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/Identity.ice \
	move Identity.h ..\..\include\icee \
	

"..\..\include\icee\Identity.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Identity.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Debug Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\Identity.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/Identity.ice \
	move Identity.h ..\..\include\icee \
	

"..\..\include\icee\Identity.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Identity.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceE\Locator.ice

!IF  "$(CFG)" == "icec - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceE\Locator.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/Locator.ice \
	move Locator.h ..\..\include\icee \
	

"..\..\include\icee\Locator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Locator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceE\Locator.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/Locator.ice \
	move Locator.h ..\..\include\icee \
	

"..\..\include\icee\Locator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Locator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Release Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\Locator.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/Locator.ice \
	move Locator.h ..\..\include\icee \
	

"..\..\include\icee\Locator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Locator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Debug Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\Locator.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/Locator.ice \
	move Locator.h ..\..\include\icee \
	

"..\..\include\icee\Locator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Locator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceE\LocatorF.ice

!IF  "$(CFG)" == "icec - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceE\LocatorF.ice

"..\..\include\icee\LocatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/LocatorF.ice 
	move LocatorF.h ..\..\include\icee 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceE\LocatorF.ice

"..\..\include\icee\LocatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/LocatorF.ice 
	move LocatorF.h ..\..\include\icee 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Release Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\LocatorF.ice

"..\..\include\icee\LocatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/LocatorF.ice 
	move LocatorF.h ..\..\include\icee 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Debug Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\LocatorF.ice

"..\..\include\icee\LocatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/LocatorF.ice 
	move LocatorF.h ..\..\include\icee 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceE\Logger.ice

!IF  "$(CFG)" == "icec - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceE\Logger.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/Logger.ice \
	move Logger.h ..\..\include\icee \
	

"..\..\include\icee\Logger.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Logger.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceE\Logger.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/Logger.ice \
	move Logger.h ..\..\include\icee \
	

"..\..\include\icee\Logger.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Logger.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Release Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\Logger.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/Logger.ice \
	move Logger.h ..\..\include\icee \
	

"..\..\include\icee\Logger.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Logger.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Debug Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\Logger.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/Logger.ice \
	move Logger.h ..\..\include\icee \
	

"..\..\include\icee\Logger.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Logger.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceE\LoggerF.ice

!IF  "$(CFG)" == "icec - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceE\LoggerF.ice

"..\..\include\icee\LoggerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/LoggerF.ice 
	move LoggerF.h ..\..\include\icee 
	del LoggerF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceE\LoggerF.ice

"..\..\include\icee\LoggerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/LoggerF.ice 
	move LoggerF.h ..\..\include\icee 
	del LoggerF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Release Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\LoggerF.ice

"..\..\include\icee\LoggerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/LoggerF.ice 
	move LoggerF.h ..\..\include\icee 
	del LoggerF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Debug Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\LoggerF.ice

"..\..\include\icee\LoggerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/LoggerF.ice 
	move LoggerF.h ..\..\include\icee 
	del LoggerF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceE\Router.ice

!IF  "$(CFG)" == "icec - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceE\Router.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/Router.ice \
	move Router.h ..\..\include\icee \
	

"..\..\include\icee\Router.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Router.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceE\Router.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/Router.ice \
	move Router.h ..\..\include\icee \
	

"..\..\include\icee\Router.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Router.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Release Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\Router.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/Router.ice \
	move Router.h ..\..\include\icee \
	

"..\..\include\icee\Router.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Router.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Debug Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\Router.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/Router.ice \
	move Router.h ..\..\include\icee \
	

"..\..\include\icee\Router.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Router.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceE\RouterF.ice

!IF  "$(CFG)" == "icec - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceE\RouterF.ice

"..\..\include\icee\RouterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/RouterF.ice 
	move RouterF.h ..\..\include\icee 
	del RouterF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceE\RouterF.ice

"..\..\include\icee\RouterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/RouterF.ice 
	move RouterF.h ..\..\include\icee 
	del RouterF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Release Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\RouterF.ice

"..\..\include\icee\RouterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/RouterF.ice 
	move RouterF.h ..\..\include\icee 
	del RouterF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "icec - Win32 Debug Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\RouterF.ice

"..\..\include\icee\RouterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/RouterF.ice 
	move RouterF.h ..\..\include\icee 
	del RouterF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
