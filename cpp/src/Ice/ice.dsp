# Microsoft Developer Studio Project File - Name="ice" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ice - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ice.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ice.mak" CFG="Ice - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ice - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ice - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ice - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I ".." /I "../../include" /D "NDEBUG" /D FD_SETSIZE=1024 /D "_USRDLL" /D "ICE_API_EXPORTS" /D "ICE_PROTOCOL_API_EXPORTS" /D "_CONSOLE" /FD /c
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
# ADD LINK32 ws2_32.lib libbz2.lib advapi32.lib /nologo /entry:"ice_DLL_Main" /dll /machine:I386 /out:"Release/ice21.dll" /implib:"Release/ice.lib"
# SUBTRACT LINK32 /pdb:none /debug /nodefaultlib
# Begin Special Build Tool
OutDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\ice.lib ..\..\lib	copy $(OutDir)\ice21.dll ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "_DEBUG" /D "_USRDLL" /D "ICE_API_EXPORTS" /D "ICE_PROTOCOL_API_EXPORTS" /D FD_SETSIZE=1024 /D "_CONSOLE" /FD /GZ /c
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
# ADD LINK32 ws2_32.lib libbz2d.lib advapi32.lib /nologo /entry:"ice_DLL_Main" /dll /debug /machine:I386 /out:"Debug/ice21d.dll" /implib:"Debug/iced.lib"
# SUBTRACT LINK32 /pdb:none /nodefaultlib
# Begin Special Build Tool
OutDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\iced.lib ..\..\lib	copy $(OutDir)\ice21d.pdb ..\..\bin	copy $(OutDir)\ice21d.dll ..\..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "ice - Win32 Release"
# Name "ice - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Acceptor.cpp
# End Source File
# Begin Source File

SOURCE=.\Application.cpp
# End Source File
# Begin Source File

SOURCE=.\BasicStream.cpp
# End Source File
# Begin Source File

SOURCE=.\Buffer.cpp
# End Source File
# Begin Source File

SOURCE=.\BuiltinSequences.cpp
# End Source File
# Begin Source File

SOURCE=.\Communicator.cpp
# End Source File
# Begin Source File

SOURCE=.\CommunicatorI.cpp
# End Source File
# Begin Source File

SOURCE=.\Connection.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnectionFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnectionI.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnectionMonitor.cpp
# End Source File
# Begin Source File

SOURCE=.\Connector.cpp
# End Source File
# Begin Source File

SOURCE=.\Current.cpp
# End Source File
# Begin Source File

SOURCE=.\DefaultsAndOverrides.cpp
# End Source File
# Begin Source File

SOURCE=.\Direct.cpp
# End Source File
# Begin Source File

SOURCE=.\DLLMain.cpp
# End Source File
# Begin Source File

SOURCE=.\DynamicLibrary.cpp
# End Source File
# Begin Source File

SOURCE=.\Endpoint.cpp
# End Source File
# Begin Source File

SOURCE=.\EndpointFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\EndpointFactoryManager.cpp
# End Source File
# Begin Source File

SOURCE=.\EventHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\EventLoggerI.cpp
# End Source File
# Begin Source File

SOURCE=.\EventLoggerMsg.mc
# End Source File
# Begin Source File

SOURCE=.\EventLoggerMsg.res
# End Source File
# Begin Source File

SOURCE=.\Exception.cpp
# End Source File
# Begin Source File

SOURCE=.\FactoryTable.cpp
# End Source File
# Begin Source File

SOURCE=.\FactoryTableDef.cpp
# End Source File
# Begin Source File

SOURCE=.\Identity.cpp
# End Source File
# Begin Source File

SOURCE=.\IdentityUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\Incoming.cpp
# End Source File
# Begin Source File

SOURCE=.\IncomingAsync.cpp
# End Source File
# Begin Source File

SOURCE=.\Initialize.cpp
# End Source File
# Begin Source File

SOURCE=.\Instance.cpp
# End Source File
# Begin Source File

SOURCE=.\LocalException.cpp
# End Source File
# Begin Source File

SOURCE=.\LocalObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Locator.cpp
# End Source File
# Begin Source File

SOURCE=.\LocatorInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Logger.cpp
# End Source File
# Begin Source File

SOURCE=.\LoggerI.cpp
# End Source File
# Begin Source File

SOURCE=.\LoggerUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\Network.cpp
# End Source File
# Begin Source File

SOURCE=.\Object.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectAdapter.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectAdapterFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectAdapterI.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectFactoryManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Outgoing.cpp
# End Source File
# Begin Source File

SOURCE=.\OutgoingAsync.cpp
# End Source File
# Begin Source File

SOURCE=.\Plugin.cpp
# End Source File
# Begin Source File

SOURCE=.\PluginManagerI.cpp
# End Source File
# Begin Source File

SOURCE=.\Process.cpp
# End Source File
# Begin Source File

SOURCE=.\Properties.cpp
# End Source File
# Begin Source File

SOURCE=.\PropertiesI.cpp
# End Source File
# Begin Source File

SOURCE=.\PropertyNames.cpp
# End Source File
# Begin Source File

SOURCE=.\ProtocolPluginFacade.cpp
# End Source File
# Begin Source File

SOURCE=.\Proxy.cpp
# End Source File
# Begin Source File

SOURCE=.\ProxyFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\Reference.cpp
# End Source File
# Begin Source File

SOURCE=.\ReferenceFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\Router.cpp
# End Source File
# Begin Source File

SOURCE=.\RouterInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\RoutingTable.cpp
# End Source File
# Begin Source File

SOURCE=.\ServantLocator.cpp
# End Source File
# Begin Source File

SOURCE=.\ServantManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Service.cpp
# End Source File
# Begin Source File

SOURCE=.\SliceChecksumDict.cpp
# End Source File
# Begin Source File

SOURCE=.\SliceChecksums.cpp
# End Source File
# Begin Source File

SOURCE=.\Stats.cpp
# End Source File
# Begin Source File

SOURCE=.\Stream.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamI.cpp
# End Source File
# Begin Source File

SOURCE=.\TcpAcceptor.cpp
# End Source File
# Begin Source File

SOURCE=.\TcpConnector.cpp
# End Source File
# Begin Source File

SOURCE=.\TcpEndpoint.cpp
# End Source File
# Begin Source File

SOURCE=.\TcpTransceiver.cpp
# End Source File
# Begin Source File

SOURCE=.\ThreadPool.cpp
# End Source File
# Begin Source File

SOURCE=.\TraceLevels.cpp
# End Source File
# Begin Source File

SOURCE=.\TraceUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\Transceiver.cpp
# End Source File
# Begin Source File

SOURCE=.\UdpEndpoint.cpp
# End Source File
# Begin Source File

SOURCE=.\UdpTransceiver.cpp
# End Source File
# Begin Source File

SOURCE=.\UnknownEndpoint.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Acceptor.h
# End Source File
# Begin Source File

SOURCE=.\AcceptorF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Application.h
# End Source File
# Begin Source File

SOURCE=.\BaseCerts.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\BasicStream.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Buffer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Communicator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\CommunicatorF.h
# End Source File
# Begin Source File

SOURCE=.\CommunicatorI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Config.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Connection.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ConnectionF.h
# End Source File
# Begin Source File

SOURCE=.\ConnectionFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ConnectionFactoryF.h
# End Source File
# Begin Source File

SOURCE=.\ConnectionI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ConnectionIF.h
# End Source File
# Begin Source File

SOURCE=.\ConnectionMonitor.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ConnectionMonitorF.h
# End Source File
# Begin Source File

SOURCE=.\Connector.h
# End Source File
# Begin Source File

SOURCE=.\ConnectorF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Current.h
# End Source File
# Begin Source File

SOURCE=.\DefaultsAndOverrides.h
# End Source File
# Begin Source File

SOURCE=.\DefaultsAndOverridesF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Direct.h
# End Source File
# Begin Source File

SOURCE=.\Endpoint.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\EndpointF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\EndpointFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\EndpointFactoryF.h
# End Source File
# Begin Source File

SOURCE=.\EndpointFactoryManager.h
# End Source File
# Begin Source File

SOURCE=.\EndpointFactoryManagerF.h
# End Source File
# Begin Source File

SOURCE=.\EventHandler.h
# End Source File
# Begin Source File

SOURCE=.\EventHandlerF.h
# End Source File
# Begin Source File

SOURCE=.\EventLoggerI.h
# End Source File
# Begin Source File

SOURCE=.\EventLoggerMsg.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Exception.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\FactoryTable.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\FactoryTableDef.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Functional.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Handle.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Ice.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Identity.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\IdentityUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Incoming.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\IncomingAsync.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\IncomingAsyncF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Initialize.h
# End Source File
# Begin Source File

SOURCE=.\Instance.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\InstanceF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\LocalException.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\LocalObject.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\LocalObjectF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Locator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\LocatorF.h
# End Source File
# Begin Source File

SOURCE=.\LocatorInfo.h
# End Source File
# Begin Source File

SOURCE=.\LocatorInfoF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Logger.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\LoggerF.h
# End Source File
# Begin Source File

SOURCE=.\LoggerI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\LoggerUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Native.h
# End Source File
# Begin Source File

SOURCE=.\Network.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Object.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ObjectAdapter.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ObjectAdapterF.h
# End Source File
# Begin Source File

SOURCE=.\ObjectAdapterFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ObjectAdapterFactoryF.h
# End Source File
# Begin Source File

SOURCE=.\ObjectAdapterI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ObjectF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ObjectFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ObjectFactoryF.h
# End Source File
# Begin Source File

SOURCE=.\ObjectFactoryManager.h
# End Source File
# Begin Source File

SOURCE=.\ObjectFactoryManagerF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Outgoing.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\OutgoingAsync.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\OutgoingAsyncF.h
# End Source File
# Begin Source File

SOURCE=.\PicklerI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Plugin.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\PluginF.h
# End Source File
# Begin Source File

SOURCE=.\PluginManagerI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Process.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ProcessF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Properties.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\PropertiesF.h
# End Source File
# Begin Source File

SOURCE=.\PropertiesI.h
# End Source File
# Begin Source File

SOURCE=.\Protocol.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ProtocolPluginFacade.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ProtocolPluginFacadeF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Proxy.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ProxyF.h
# End Source File
# Begin Source File

SOURCE=.\ProxyFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ProxyFactoryF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ProxyHandle.h
# End Source File
# Begin Source File

SOURCE=.\Reference.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ReferenceF.h
# End Source File
# Begin Source File

SOURCE=.\ReferenceFactory.h
# End Source File
# Begin Source File

SOURCE=.\ReferenceFactoryF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Router.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\RouterF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\RouterInfo.h
# End Source File
# Begin Source File

SOURCE=.\RouterInfo.h
# End Source File
# Begin Source File

SOURCE=.\RouterInfoF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\RoutingTable.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\RoutingTableF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ServantLocator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ServantLocatorF.h
# End Source File
# Begin Source File

SOURCE=.\ServantManager.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\ServantManagerF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Service.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\SliceChecksumDict.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\SliceChecksums.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Stats.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\StatsF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\Stream.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\StreamF.h
# End Source File
# Begin Source File

SOURCE=.\StreamI.h
# End Source File
# Begin Source File

SOURCE=.\TcpAcceptor.h
# End Source File
# Begin Source File

SOURCE=.\TcpConnector.h
# End Source File
# Begin Source File

SOURCE=.\TcpEndpoint.h
# End Source File
# Begin Source File

SOURCE=.\TcpTransceiver.h
# End Source File
# Begin Source File

SOURCE=.\ThreadPool.h
# End Source File
# Begin Source File

SOURCE=.\ThreadPoolF.h
# End Source File
# Begin Source File

SOURCE=.\TraceLevels.h
# End Source File
# Begin Source File

SOURCE=.\TraceLevelsF.h
# End Source File
# Begin Source File

SOURCE=.\TraceUtil.h
# End Source File
# Begin Source File

SOURCE=.\Transceiver.h
# End Source File
# Begin Source File

SOURCE=.\TransceiverF.h
# End Source File
# Begin Source File

SOURCE=.\UdpEndpoint.h
# End Source File
# Begin Source File

SOURCE=.\UdpTransceiver.h
# End Source File
# Begin Source File

SOURCE=.\UnknownEndpoint.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\UserExceptionFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ice\UserExceptionFactoryF.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\slice\Ice\BuiltinSequences.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__BUILT="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\BuiltinSequences.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/BuiltinSequences.ice \
	move BuiltinSequences.h. ..\..\include\ice \
	

"..\..\include\ice\BuiltinSequences.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"BuiltinSequences.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__BUILT="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\BuiltinSequences.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/BuiltinSequences.ice \
	move BuiltinSequences.h. ..\..\include\ice \
	

"..\..\include\ice\BuiltinSequences.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"BuiltinSequences.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Communicator.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__COMMU="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Communicator.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice -DICE_CPP --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Communicator.ice \
	move Communicator.h ..\..\include\ice \
	

"..\..\include\ice\Communicator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Communicator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__COMMU="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Communicator.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice -DICE_CPP --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Communicator.ice \
	move Communicator.h ..\..\include\ice \
	

"..\..\include\ice\Communicator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Communicator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\CommunicatorF.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__COMMUN="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\CommunicatorF.ice

"..\..\include\ice\CommunicatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/CommunicatorF.ice 
	move CommunicatorF.h ..\..\include\ice 
	del CommunicatorF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__COMMUN="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\CommunicatorF.ice

"..\..\include\ice\CommunicatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/CommunicatorF.ice 
	move CommunicatorF.h ..\..\include\ice 
	del CommunicatorF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Connection.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__CONNE="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Connection.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice -DICE_CPP --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Connection.ice \
	move Connection.h ..\..\include\ice \
	

"..\..\include\ice\Connection.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Connection.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__CONNE="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Connection.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice -DICE_CPP --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Connection.ice \
	move Connection.h ..\..\include\ice \
	

"..\..\include\ice\Connection.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Connection.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\ConnectionF.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__CONNEC="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ConnectionF.ice

"..\..\include\ice\ConnectionF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ConnectionF.ice 
	move ConnectionF.h ..\..\include\ice 
	del ConnectionF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__CONNEC="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ConnectionF.ice

"..\..\include\ice\ConnectionF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ConnectionF.ice 
	move ConnectionF.h ..\..\include\ice 
	del ConnectionF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Current.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__CURRE="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Current.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Current.ice \
	move Current.h ..\..\include\ice \
	

"..\..\include\ice\Current.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Current.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__CURRE="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Current.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Current.ice \
	move Current.h ..\..\include\ice \
	

"..\..\include\ice\Current.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Current.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\EventLoggerMsg.mc

!IF  "$(CFG)" == "ice - Win32 Release"

# Begin Custom Build
InputPath=.\EventLoggerMsg.mc
InputName=EventLoggerMsg

BuildCmds= \
	mc $(InputPath) \
	rc -r -fo $(InputName).res $(InputName).rc \
	

"EventLoggerMsg.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"EventLoggerMsg.res" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

# Begin Custom Build
InputPath=.\EventLoggerMsg.mc
InputName=EventLoggerMsg

BuildCmds= \
	mc $(InputPath) \
	rc -r -fo $(InputName).res $(InputName).rc \
	

"EventLoggerMsg.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"EventLoggerMsg.res" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\FacetMap.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__FACET="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\FacetMap.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/FacetMap.ice \
	move FacetMap.h ..\..\include\ice \
	

"..\..\include\ice\FacetMap.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"FacetMap.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__FACET="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\FacetMap.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/FacetMap.ice \
	move FacetMap.h ..\..\include\ice \
	

"..\..\include\ice\FacetMap.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"FacetMap.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Identity.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__IDENT="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Identity.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Identity.ice \
	move Identity.h ..\..\include\ice \
	

"..\..\include\ice\Identity.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Identity.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__IDENT="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Identity.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Identity.ice \
	move Identity.h ..\..\include\ice \
	

"..\..\include\ice\Identity.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Identity.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\LocalException.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__LOCAL="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\LocalException.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/LocalException.ice \
	move LocalException.h ..\..\include\ice \
	

"..\..\include\ice\LocalException.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LocalException.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__LOCAL="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\LocalException.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/LocalException.ice \
	move LocalException.h ..\..\include\ice \
	

"..\..\include\ice\LocalException.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LocalException.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Locator.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__LOCAT="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Locator.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Locator.ice \
	move Locator.h ..\..\include\ice \
	

"..\..\include\ice\Locator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Locator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__LOCAT="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Locator.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Locator.ice \
	move Locator.h ..\..\include\ice \
	

"..\..\include\ice\Locator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Locator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\LocatorF.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__LOCATO="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\LocatorF.ice

"..\..\include\ice\LocatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/LocatorF.ice 
	move LocatorF.h ..\..\include\ice 
	del LocatorF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__LOCATO="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\LocatorF.ice

"..\..\include\ice\LocatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/LocatorF.ice 
	move LocatorF.h ..\..\include\ice 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Logger.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__LOGGE="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Logger.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Logger.ice \
	move Logger.h ..\..\include\ice \
	

"..\..\include\ice\Logger.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Logger.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__LOGGE="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Logger.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Logger.ice \
	move Logger.h ..\..\include\ice \
	

"..\..\include\ice\Logger.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Logger.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\LoggerF.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__LOGGER="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\LoggerF.ice

"..\..\include\ice\LoggerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/LoggerF.ice 
	move LoggerF.h ..\..\include\ice 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__LOGGER="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\LoggerF.ice

"..\..\include\ice\LoggerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/LoggerF.ice 
	move LoggerF.h ..\..\include\ice 
	del LoggerF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\ObjectAdapter.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__OBJEC="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ObjectAdapter.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ObjectAdapter.ice \
	move ObjectAdapter.h ..\..\include\ice \
	

"..\..\include\ice\ObjectAdapter.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ObjectAdapter.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__OBJEC="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ObjectAdapter.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ObjectAdapter.ice \
	move ObjectAdapter.h ..\..\include\ice \
	

"..\..\include\ice\ObjectAdapter.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ObjectAdapter.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\ObjectAdapterF.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__OBJECT="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ObjectAdapterF.ice

"..\..\include\ice\ObjectAdapterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ObjectAdapterF.ice 
	move ObjectAdapterF.h ..\..\include\ice 
	del ObjectAdapterF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__OBJECT="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ObjectAdapterF.ice

"..\..\include\ice\ObjectAdapterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ObjectAdapterF.ice 
	move ObjectAdapterF.h ..\..\include\ice 
	del ObjectAdapterF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\ObjectFactory.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__OBJECTF="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ObjectFactory.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ObjectFactory.ice \
	move ObjectFactory.h ..\..\include\ice \
	

"..\..\include\ice\ObjectFactory.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ObjectFactory.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__OBJECTF="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ObjectFactory.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ObjectFactory.ice \
	move ObjectFactory.h ..\..\include\ice \
	

"..\..\include\ice\ObjectFactory.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ObjectFactory.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\ObjectFactoryF.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__OBJECTFA="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ObjectFactoryF.ice

"..\..\include\ice\ObjectFactoryF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ObjectFactoryF.ice 
	move ObjectFactoryF.h ..\..\include\ice 
	del ObjectFactoryF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__OBJECTFA="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ObjectFactoryF.ice

"..\..\include\ice\ObjectFactoryF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ObjectFactoryF.ice 
	move ObjectFactoryF.h ..\..\include\ice 
	del ObjectFactoryF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Plugin.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__PLUGI="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Plugin.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Plugin.ice \
	move Plugin.h ..\..\include\ice \
	

"..\..\include\ice\Plugin.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Plugin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__PLUGI="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Plugin.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Plugin.ice \
	move Plugin.h ..\..\include\ice \
	

"..\..\include\ice\Plugin.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Plugin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\PluginF.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__PLUGIN="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\PluginF.ice

"..\..\include\ice\PluginF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/PluginF.ice 
	move PluginF.h ..\..\include\ice 
	del PluginF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__PLUGIN="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\PluginF.ice

"..\..\include\ice\PluginF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/PluginF.ice 
	move PluginF.h ..\..\include\ice 
	del PluginF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Process.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__PROCE="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Process.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Process.ice \
	move Process.h ..\..\include\ice \
	

"..\..\include\ice\Process.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Process.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__PROCE="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Process.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Process.ice \
	move Process.h ..\..\include\ice \
	

"..\..\include\ice\Process.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Process.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\ProcessF.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__PROCES="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ProcessF.ice

"..\..\include\ice\ProcessF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ProcessF.ice 
	move ProcessF.h ..\..\include\ice 
	del ProcessF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__PROCES="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ProcessF.ice

"..\..\include\ice\ProcessF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ProcessF.ice 
	move ProcessF.h ..\..\include\ice 
	del ProcessF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Properties.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__PROPE="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Properties.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Properties.ice \
	move Properties.h ..\..\include\ice \
	

"..\..\include\ice\Properties.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Properties.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__PROPE="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Properties.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Properties.ice \
	move Properties.h ..\..\include\ice \
	

"..\..\include\ice\Properties.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Properties.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\PropertiesF.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__PROPER="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\PropertiesF.ice

"..\..\include\ice\PropertiesF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/PropertiesF.ice 
	move PropertiesF.h ..\..\include\ice 
	del PropertiesF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__PROPER="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\PropertiesF.ice

"..\..\include\ice\PropertiesF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/PropertiesF.ice 
	move PropertiesF.h ..\..\include\ice 
	del PropertiesF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Router.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__ROUTE="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Router.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Router.ice \
	move Router.h ..\..\include\ice \
	

"..\..\include\ice\Router.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Router.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__ROUTE="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Router.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Router.ice \
	move Router.h ..\..\include\ice \
	

"..\..\include\ice\Router.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Router.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\RouterF.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__ROUTER="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\RouterF.ice

"..\..\include\ice\RouterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/RouterF.ice 
	move RouterF.h ..\..\include\ice 
	del RouterF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__ROUTER="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\RouterF.ice

"..\..\include\ice\RouterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/RouterF.ice 
	move RouterF.h ..\..\include\ice 
	del RouterF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\ServantLocator.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__SERVA="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ServantLocator.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ServantLocator.ice \
	move ServantLocator.h ..\..\include\ice \
	

"..\..\include\ice\ServantLocator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ServantLocator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__SERVA="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ServantLocator.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ServantLocator.ice \
	move ServantLocator.h ..\..\include\ice \
	

"..\..\include\ice\ServantLocator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ServantLocator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\ServantLocatorF.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__SERVAN="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ServantLocatorF.ice

"..\..\include\ice\ServantLocatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ServantLocatorF.ice 
	move ServantLocatorF.h ..\..\include\ice 
	del ServantLocatorF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__SERVAN="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ServantLocatorF.ice

"..\..\include\ice\ServantLocatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ServantLocatorF.ice 
	move ServantLocatorF.h ..\..\include\ice 
	del ServantLocatorF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\SliceChecksumDict.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__SLice="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\SliceChecksumDict.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/SliceChecksumDict.ice \
	move SliceChecksumDict.h ..\..\include\Ice \
	

"..\..\include\ice\SliceChecksumDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"SliceChecksumDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__SLice="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\SliceChecksumDict.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/SliceChecksumDict.ice \
	move SliceChecksumDict.h ..\..\include\Ice \
	

"..\..\include\ice\SliceChecksumDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"SliceChecksumDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Stats.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__STATS="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Stats.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Stats.ice \
	move Stats.h ..\..\include\ice \
	

"..\..\include\ice\Stats.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Stats.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__STATS="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Stats.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Stats.ice \
	move Stats.h ..\..\include\ice \
	

"..\..\include\ice\Stats.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Stats.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\StatsF.ice

!IF  "$(CFG)" == "ice - Win32 Release"

USERDEP__STATSF="..\..\bin\slice2cpp.exe"	"..\..\lib\slice.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\StatsF.ice

"..\..\include\ice\StatsF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/StatsF.ice 
	move StatsF.h ..\..\include\ice 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

USERDEP__STATSF="..\..\bin\slice2cpp.exe"	"..\..\lib\sliced.lib"	
# Begin Custom Build
InputPath=..\..\slice\Ice\StatsF.ice

"..\..\include\ice\StatsF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/StatsF.ice 
	move StatsF.h ..\..\include\ice 
	del StatsF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
