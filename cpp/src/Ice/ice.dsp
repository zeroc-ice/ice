# Microsoft Developer Studio Project File - Name="Ice" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Ice - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Ice.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Ice.mak" CFG="Ice - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Ice - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Ice - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Ice - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I ".." /I "../../include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_USRDLL" /D "ICE_API_EXPORTS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ws2_32.lib jtc.lib /nologo /dll /machine:I386 /out:"Release/ice001.dll"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Release\ice001.* ..\..\lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_USRDLL" /D "ICE_API_EXPORTS" /YX /FD /GZ /c
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
# ADD LINK32 ws2_32.lib jtcd.lib /nologo /dll /debug /machine:I386 /out:"Debug/ice001d.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Debug\ice001d.* ..\..\lib
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Ice - Win32 Release"
# Name "Ice - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Acceptor.cpp
# End Source File
# Begin Source File

SOURCE=.\Application.cpp
# End Source File
# Begin Source File

SOURCE=.\Collector.cpp
# End Source File
# Begin Source File

SOURCE=.\Communicator.cpp
# End Source File
# Begin Source File

SOURCE=.\CommunicatorI.cpp
# End Source File
# Begin Source File

SOURCE=.\Connector.cpp
# End Source File
# Begin Source File

SOURCE=.\Direct.cpp
# End Source File
# Begin Source File

SOURCE=.\Emitter.cpp
# End Source File
# Begin Source File

SOURCE=.\Endpoint.cpp
# End Source File
# Begin Source File

SOURCE=.\EventHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\Incoming.cpp
# End Source File
# Begin Source File

SOURCE=.\Instance.cpp
# End Source File
# Begin Source File

SOURCE=.\Exception.cpp
# End Source File
# Begin Source File

SOURCE=.\LocalException.cpp
# End Source File
# Begin Source File

SOURCE=.\LocalObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Logger.cpp
# End Source File
# Begin Source File

SOURCE=.\LoggerI.cpp
# End Source File
# Begin Source File

SOURCE=.\Stream.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamI.cpp
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

SOURCE=.\Outgoing.cpp
# End Source File
# Begin Source File

SOURCE=.\Properties.cpp
# End Source File
# Begin Source File

SOURCE=.\PropertiesI.cpp
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

SOURCE=.\ServantFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\ServantFactoryManager.cpp
# End Source File
# Begin Source File

SOURCE=.\SslAcceptor.cpp
# End Source File
# Begin Source File

SOURCE=.\SslConnector.cpp
# End Source File
# Begin Source File

SOURCE=.\SslTransceiver.cpp
# End Source File
# Begin Source File

SOURCE=.\BasicStream.cpp
# End Source File
# Begin Source File

SOURCE=.\TcpAcceptor.cpp
# End Source File
# Begin Source File

SOURCE=.\TcpConnector.cpp
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

SOURCE=.\UdpTransceiver.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Ice\Acceptor.h
# End Source File
# Begin Source File

SOURCE=.\Ice\AcceptorF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Application.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Buffer.h
# End Source File
# Begin Source File

SOURCE=.\Ice\Collector.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\CollectorF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Communicator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\CommunicatorF.h
# End Source File
# Begin Source File

SOURCE=.\Ice\CommunicatorI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Config.h
# End Source File
# Begin Source File

SOURCE=.\Ice\Connector.h
# End Source File
# Begin Source File

SOURCE=.\Ice\ConnectorF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Direct.h
# End Source File
# Begin Source File

SOURCE=.\Ice\Emitter.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\EmitterF.h
# End Source File
# Begin Source File

SOURCE=.\Ice\Endpoint.h
# End Source File
# Begin Source File

SOURCE=.\Ice\EndpointF.h
# End Source File
# Begin Source File

SOURCE=.\Ice\EventHandler.h
# End Source File
# Begin Source File

SOURCE=.\Ice\EventHandlerF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Functional.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Handle.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Ice.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Incoming.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Initialize.h
# End Source File
# Begin Source File

SOURCE=.\Ice\Instance.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\InstanceF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Exception.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\LocalException.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\LocalObject.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\LocalObjectF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Logger.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\LoggerF.h
# End Source File
# Begin Source File

SOURCE=.\Ice\LoggerI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Stream.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\StreamF.h
# End Source File
# Begin Source File

SOURCE=.\Ice\StreamI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Native.h
# End Source File
# Begin Source File

SOURCE=.\Ice\Network.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Object.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ObjectAdapter.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ObjectAdapterF.h
# End Source File
# Begin Source File

SOURCE=.\Ice\ObjectAdapterFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ObjectAdapterFactoryF.h
# End Source File
# Begin Source File

SOURCE=.\Ice\ObjectAdapterI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ObjectF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Outgoing.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Properties.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\PropertiesF.h
# End Source File
# Begin Source File

SOURCE=.\Ice\PropertiesI.h
# End Source File
# Begin Source File

SOURCE=.\Protocol.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Proxy.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ProxyF.h
# End Source File
# Begin Source File

SOURCE=.\Ice\ProxyFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ProxyFactoryF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ProxyHandle.h
# End Source File
# Begin Source File

SOURCE=.\Ice\Reference.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ReferenceF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ServantFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ServantFactoryF.h
# End Source File
# Begin Source File

SOURCE=.\Ice\ServantFactoryManager.h
# End Source File
# Begin Source File

SOURCE=.\Ice\ServantFactoryManagerF.h
# End Source File
# Begin Source File

SOURCE=.\Ice\SslAcceptor.h
# End Source File
# Begin Source File

SOURCE=.\Ice\SslConnector.h
# End Source File
# Begin Source File

SOURCE=.\Ice\SslTransceiver.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\BasicStream.h
# End Source File
# Begin Source File

SOURCE=.\Ice\TcpAcceptor.h
# End Source File
# Begin Source File

SOURCE=.\Ice\TcpConnector.h
# End Source File
# Begin Source File

SOURCE=.\Ice\TcpTransceiver.h
# End Source File
# Begin Source File

SOURCE=.\Ice\ThreadPool.h
# End Source File
# Begin Source File

SOURCE=.\Ice\ThreadPoolF.h
# End Source File
# Begin Source File

SOURCE=.\Ice\TraceLevels.h
# End Source File
# Begin Source File

SOURCE=.\Ice\TraceLevelsF.h
# End Source File
# Begin Source File

SOURCE=.\Ice\TraceUtil.h
# End Source File
# Begin Source File

SOURCE=.\Ice\Transceiver.h
# End Source File
# Begin Source File

SOURCE=.\Ice\TransceiverF.h
# End Source File
# Begin Source File

SOURCE=.\Ice\UdpTransceiver.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\slice\Ice\LocalException.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\Ice\LocalException.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/LocalException.ice \
	move LocalException.h ..\..\include\Ice \
	

"..\..\include\Ice\LocalException.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LocalException.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\Ice\LocalException.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/LocalException.ice \
	move LocalException.h ..\..\include\Ice \
	

"..\..\include\Ice\LocalException.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LocalException.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Communicator.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\Ice\Communicator.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Communicator.ice \
	move Communicator.h ..\..\include\Ice \
	

"..\..\include\Ice\Communicator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Communicator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\Ice\Communicator.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Communicator.ice \
	move Communicator.h ..\..\include\Ice \
	

"..\..\include\Ice\Communicator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Communicator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\CommunicatorF.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\Ice\CommunicatorF.ice

"..\..\include\Ice\CommunicatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/CommunicatorF.ice 
	move CommunicatorF.h ..\..\include\Ice 
	del CommunicatorF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\Ice\CommunicatorF.ice

"..\..\include\Ice\CommunicatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/CommunicatorF.ice 
	move CommunicatorF.h ..\..\include\Ice 
	del CommunicatorF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Logger.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\Ice\Logger.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Logger.ice \
	move Logger.h ..\..\include\Ice \
	

"..\..\include\Ice\Logger.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Logger.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\Ice\Logger.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Logger.ice \
	move Logger.h ..\..\include\Ice \
	

"..\..\include\Ice\Logger.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Logger.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\LoggerF.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\Ice\LoggerF.ice

"..\..\include\Ice\LoggerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/LoggerF.ice 
	move LoggerF.h ..\..\include\Ice 
	del LoggerF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\Ice\LoggerF.ice

"..\..\include\Ice\LoggerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/LoggerF.ice 
	move LoggerF.h ..\..\include\Ice 
	del LoggerF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Stream.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\Ice\Stream.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Stream.ice \
	move Stream.h ..\..\include\Ice \
	

"..\..\include\Ice\Stream.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Stream.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\Ice\Stream.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Stream.ice \
	move Stream.h ..\..\include\Ice \
	

"..\..\include\Ice\Stream.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Stream.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\StreamF.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\Ice\StreamF.ice

"..\..\include\Ice\StreamF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/StreamF.ice 
	move StreamF.h ..\..\include\Ice 
	del StreamF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\Ice\StreamF.ice

"..\..\include\Ice\StreamF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/StreamF.ice 
	move StreamF.h ..\..\include\Ice 
	del StreamF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\ObjectAdapter.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\Ice\ObjectAdapter.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ObjectAdapter.ice \
	move ObjectAdapter.h ..\..\include\Ice \
	

"..\..\include\Ice\ObjectAdapter.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ObjectAdapter.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\Ice\ObjectAdapter.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ObjectAdapter.ice \
	move ObjectAdapter.h ..\..\include\Ice \
	

"..\..\include\Ice\ObjectAdapter.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ObjectAdapter.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\ObjectAdapterF.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\Ice\ObjectAdapterF.ice

"..\..\include\Ice\ObjectAdapterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ObjectAdapterF.ice 
	move ObjectAdapterF.h ..\..\include\Ice 
	del ObjectAdapterF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\Ice\ObjectAdapterF.ice

"..\..\include\Ice\ObjectAdapterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ObjectAdapterF.ice 
	move ObjectAdapterF.h ..\..\include\Ice 
	del ObjectAdapterF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Properties.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\Ice\Properties.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Properties.ice \
	move Properties.h ..\..\include\Ice \
	

"..\..\include\Ice\Properties.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Properties.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\Ice\Properties.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Properties.ice \
	move Properties.h ..\..\include\Ice \
	

"..\..\include\Ice\Properties.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Properties.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\PropertiesF.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\Ice\PropertiesF.ice

"..\..\include\Ice\PropertiesF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/PropertiesF.ice 
	move PropertiesF.h ..\..\include\Ice 
	del PropertiesF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\Ice\PropertiesF.ice

"..\..\include\Ice\PropertiesF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/PropertiesF.ice 
	move PropertiesF.h ..\..\include\Ice 
	del PropertiesF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\ServantFactory.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\Ice\ServantFactory.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ServantFactory.ice \
	move ServantFactory.h ..\..\include\Ice \
	

"..\..\include\Ice\ServantFactory.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ServantFactory.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\Ice\ServantFactory.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ServantFactory.ice \
	move ServantFactory.h ..\..\include\Ice \
	

"..\..\include\Ice\ServantFactory.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ServantFactory.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\ServantFactoryF.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\Ice\ServantFactoryF.ice

"..\..\include\Ice\ServantFactoryF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ServantFactoryF.ice 
	move ServantFactoryF.h ..\..\include\Ice 
	del ServantFactoryF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\Ice\ServantFactoryF.ice

"..\..\include\Ice\ServantFactoryF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ServantFactoryF.ice 
	move ServantFactoryF.h ..\..\include\Ice 
	del ServantFactoryF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
