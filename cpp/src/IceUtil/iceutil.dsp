# Microsoft Developer Studio Project File - Name="IceUtil" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=IceUtil - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "IceUtil.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "IceUtil.mak" CFG="IceUtil - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "IceUtil - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "IceUtil - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "IceUtil - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I ".." /I "../../include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_USRDLL" /D "ICE_API_EXPORTS" /YX /FD /c
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

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

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
# SUBTRACT CPP /WX /Fr
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

# Name "IceUtil - Win32 Release"
# Name "IceUtil - Win32 Debug"
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

SOURCE=.\Exception.cpp
# End Source File
# Begin Source File

SOURCE=.\Incoming.cpp
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

SOURCE=.\Logger.cpp
# End Source File
# Begin Source File

SOURCE=.\LoggerI.cpp
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

SOURCE=.\ServantLocator.cpp
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
# Begin Source File

SOURCE=.\UserExceptionFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\UserExceptionFactoryManager.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\IceUtil\Acceptor.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\AcceptorF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Application.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\BasicStream.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Buffer.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\Collector.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\CollectorF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Communicator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\CommunicatorF.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\CommunicatorI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Config.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\Connector.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\ConnectorF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Direct.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\Emitter.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\EmitterF.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\Endpoint.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\EndpointF.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\EventHandler.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\EventHandlerF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Exception.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Functional.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Handle.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\IceUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Incoming.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Initialize.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\Instance.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\InstanceF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\LocalException.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\LocalObject.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\LocalObjectF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Logger.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\LoggerF.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\LoggerI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Native.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\Network.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Object.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\ObjectAdapter.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\ObjectAdapterF.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\ObjectAdapterFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\ObjectAdapterFactoryF.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\ObjectAdapterI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\ObjectF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Outgoing.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Properties.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\PropertiesF.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\PropertiesI.h
# End Source File
# Begin Source File

SOURCE=.\Protocol.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Proxy.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\ProxyF.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\ProxyFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\ProxyFactoryF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\ProxyHandle.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\Reference.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\ReferenceF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\ServantFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\ServantFactoryF.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\ServantFactoryManager.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\ServantFactoryManagerF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\ServantLocator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\ServantLocatorF.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\SslAcceptor.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\SslConnector.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\SslTransceiver.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\Stream.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\StreamF.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\StreamI.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\TcpAcceptor.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\TcpConnector.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\TcpTransceiver.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\ThreadPool.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\ThreadPoolF.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\TraceLevels.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\TraceLevelsF.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\TraceUtil.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\Transceiver.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\TransceiverF.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\UdpTransceiver.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\UserExceptionFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceUtil\UserExceptionFactoryF.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\UserExceptionFactoryManager.h
# End Source File
# Begin Source File

SOURCE=.\IceUtil\UserExceptionFactoryManagerF.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\slice\IceUtil\Communicator.ice

!IF  "$(CFG)" == "IceUtil - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\Communicator.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/Communicator.ice \
	move Communicator.h ..\..\include\IceUtil \
	

"..\..\include\IceUtil\Communicator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Communicator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\Communicator.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/Communicator.ice \
	move Communicator.h ..\..\include\IceUtil \
	

"..\..\include\IceUtil\Communicator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Communicator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceUtil\CommunicatorF.ice

!IF  "$(CFG)" == "IceUtil - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\CommunicatorF.ice

"..\..\include\IceUtil\CommunicatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/CommunicatorF.ice 
	move CommunicatorF.h ..\..\include\IceUtil 
	del CommunicatorF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\CommunicatorF.ice

"..\..\include\IceUtil\CommunicatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/CommunicatorF.ice 
	move CommunicatorF.h ..\..\include\IceUtil 
	del CommunicatorF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceUtil\LocalException.ice

!IF  "$(CFG)" == "IceUtil - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\LocalException.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/LocalException.ice \
	move LocalException.h ..\..\include\IceUtil \
	

"..\..\include\IceUtil\LocalException.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LocalException.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\LocalException.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/LocalException.ice \
	move LocalException.h ..\..\include\IceUtil \
	

"..\..\include\IceUtil\LocalException.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LocalException.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceUtil\Logger.ice

!IF  "$(CFG)" == "IceUtil - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\Logger.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/Logger.ice \
	move Logger.h ..\..\include\IceUtil \
	

"..\..\include\IceUtil\Logger.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Logger.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\Logger.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/Logger.ice \
	move Logger.h ..\..\include\IceUtil \
	

"..\..\include\IceUtil\Logger.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Logger.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceUtil\LoggerF.ice

!IF  "$(CFG)" == "IceUtil - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\LoggerF.ice

"..\..\include\IceUtil\LoggerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/LoggerF.ice 
	move LoggerF.h ..\..\include\IceUtil 
	del LoggerF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\LoggerF.ice

"..\..\include\IceUtil\LoggerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/LoggerF.ice 
	move LoggerF.h ..\..\include\IceUtil 
	del LoggerF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceUtil\ObjectAdapter.ice

!IF  "$(CFG)" == "IceUtil - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\ObjectAdapter.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/ObjectAdapter.ice \
	move ObjectAdapter.h ..\..\include\IceUtil \
	

"..\..\include\IceUtil\ObjectAdapter.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ObjectAdapter.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\ObjectAdapter.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/ObjectAdapter.ice \
	move ObjectAdapter.h ..\..\include\IceUtil \
	

"..\..\include\IceUtil\ObjectAdapter.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ObjectAdapter.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceUtil\ObjectAdapterF.ice

!IF  "$(CFG)" == "IceUtil - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\ObjectAdapterF.ice

"..\..\include\IceUtil\ObjectAdapterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/ObjectAdapterF.ice 
	move ObjectAdapterF.h ..\..\include\IceUtil 
	del ObjectAdapterF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\ObjectAdapterF.ice

"..\..\include\IceUtil\ObjectAdapterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/ObjectAdapterF.ice 
	move ObjectAdapterF.h ..\..\include\IceUtil 
	del ObjectAdapterF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceUtil\Properties.ice

!IF  "$(CFG)" == "IceUtil - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\Properties.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/Properties.ice \
	move Properties.h ..\..\include\IceUtil \
	

"..\..\include\IceUtil\Properties.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Properties.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\Properties.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/Properties.ice \
	move Properties.h ..\..\include\IceUtil \
	

"..\..\include\IceUtil\Properties.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Properties.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceUtil\PropertiesF.ice

!IF  "$(CFG)" == "IceUtil - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\PropertiesF.ice

"..\..\include\IceUtil\PropertiesF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/PropertiesF.ice 
	move PropertiesF.h ..\..\include\IceUtil 
	del PropertiesF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\PropertiesF.ice

"..\..\include\IceUtil\PropertiesF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/PropertiesF.ice 
	move PropertiesF.h ..\..\include\IceUtil 
	del PropertiesF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceUtil\ServantFactory.ice

!IF  "$(CFG)" == "IceUtil - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\ServantFactory.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/ServantFactory.ice \
	move ServantFactory.h ..\..\include\IceUtil \
	

"..\..\include\IceUtil\ServantFactory.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ServantFactory.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\ServantFactory.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/ServantFactory.ice \
	move ServantFactory.h ..\..\include\IceUtil \
	

"..\..\include\IceUtil\ServantFactory.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ServantFactory.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceUtil\ServantFactoryF.ice

!IF  "$(CFG)" == "IceUtil - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\ServantFactoryF.ice

"..\..\include\IceUtil\ServantFactoryF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/ServantFactoryF.ice 
	move ServantFactoryF.h ..\..\include\IceUtil 
	del ServantFactoryF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\ServantFactoryF.ice

"..\..\include\IceUtil\ServantFactoryF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/ServantFactoryF.ice 
	move ServantFactoryF.h ..\..\include\IceUtil 
	del ServantFactoryF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceUtil\ServantLocator.ice

!IF  "$(CFG)" == "IceUtil - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\ServantLocator.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/ServantLocator.ice \
	move ServantLocator.h ..\..\include\IceUtil \
	

"..\..\include\IceUtil\ServantLocator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ServantLocator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\ServantLocator.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/ServantLocator.ice \
	move ServantLocator.h ..\..\include\IceUtil \
	

"..\..\include\IceUtil\ServantLocator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ServantLocator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceUtil\ServantLocatorF.ice

!IF  "$(CFG)" == "IceUtil - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\ServantLocatorF.ice

"..\..\include\IceUtil\ServantLocatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/ServantLocatorF.ice 
	move ServantLocatorF.h ..\..\include\IceUtil 
	del ServantLocatorF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\ServantLocatorF.ice

"..\..\include\IceUtil\ServantLocatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/ServantLocatorF.ice 
	move ServantLocatorF.h ..\..\include\IceUtil 
	del ServantLocatorF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceUtil\Stream.ice

!IF  "$(CFG)" == "IceUtil - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\Stream.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/Stream.ice \
	move Stream.h ..\..\include\IceUtil \
	

"..\..\include\IceUtil\Stream.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Stream.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\Stream.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/Stream.ice \
	move Stream.h ..\..\include\IceUtil \
	

"..\..\include\IceUtil\Stream.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Stream.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceUtil\StreamF.ice

!IF  "$(CFG)" == "IceUtil - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\StreamF.ice

"..\..\include\IceUtil\StreamF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/StreamF.ice 
	move StreamF.h ..\..\include\IceUtil 
	del StreamF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\StreamF.ice

"..\..\include\IceUtil\StreamF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/StreamF.ice 
	move StreamF.h ..\..\include\IceUtil 
	del StreamF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceUtil\UserExceptionFactory.ice

!IF  "$(CFG)" == "IceUtil - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\UserExceptionFactory.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/UserExceptionFactory.ice \
	move UserExceptionFactory.h ..\..\include\IceUtil \
	

"..\..\include\IceUtil\UserExceptionFactory.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"UserExceptionFactory.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\UserExceptionFactory.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/UserExceptionFactory.ice \
	move UserExceptionFactory.h ..\..\include\IceUtil \
	

"..\..\include\IceUtil\UserExceptionFactory.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"UserExceptionFactory.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceUtil\UserExceptionFactoryF.ice

!IF  "$(CFG)" == "IceUtil - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\UserExceptionFactoryF.ice

"..\..\include\IceUtil\UserExceptionFactoryF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/UserExceptionFactoryF.ice 
	move UserExceptionFactoryF.h ..\..\include\IceUtil 
	del UserExceptionFactoryF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "IceUtil - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceUtil\UserExceptionFactoryF.ice

"..\..\include\IceUtil\UserExceptionFactoryF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IceUtil -I../../slice ../../slice/IceUtil/UserExceptionFactoryF.ice 
	move UserExceptionFactoryF.h ..\..\include\IceUtil 
	del UserExceptionFactoryF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
