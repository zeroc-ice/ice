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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I ".." /I "../../include" /D "WIN32" /D "_UNICODE" /D "NDEBUG" /D "_CONSOLE" /D "_USRDLL" /D "ICE_API_EXPORTS" /YX /FD /c
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
# ADD LINK32 ws2_32.lib libeay32.lib ssleay32.lib xerces-c_1.lib /nologo /dll /machine:I386 /out:"Release/ice001.dll"
# SUBTRACT LINK32 /pdb:none /debug /nodefaultlib
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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "WIN32" /D "_UNICODE" /D "_DEBUG" /D "_CONSOLE" /D "_USRDLL" /D "ICE_API_EXPORTS" /YX /FD /GZ /c
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
# ADD LINK32 ws2_32.lib libeay32.lib ssleay32.lib xerces-c_1D.lib /nologo /dll /debug /machine:I386 /out:"Debug/ice001d.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /nodefaultlib
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

SOURCE=.\BasicStream.cpp
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

SOURCE=.\Connector.cpp
# End Source File
# Begin Source File

SOURCE=.\CryptKey.cpp
# End Source File
# Begin Source File

SOURCE=.\Cryptor.cpp
# End Source File
# Begin Source File

SOURCE=.\Current.cpp
# End Source File
# Begin Source File

SOURCE=.\Direct.cpp
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

SOURCE=.\Identity.cpp
# End Source File
# Begin Source File

SOURCE=.\IdentityUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\Incoming.cpp
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

SOURCE=.\Logger.cpp
# End Source File
# Begin Source File

SOURCE=.\LoggerUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\LoggerI.cpp
# End Source File
# Begin Source File

SOURCE=.\MessageAuthenticator.cpp
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

SOURCE=.\SecureUdp.cpp
# End Source File
# Begin Source File

SOURCE=.\SecurityException.cpp
# End Source File
# Begin Source File

SOURCE=.\SecurityException2.cpp
# End Source File
# Begin Source File

SOURCE=.\ServantLocator.cpp
# End Source File
# Begin Source File

SOURCE=.\SslAcceptor.cpp
# End Source File
# Begin Source File

SOURCE=.\SslBaseCerts.cpp
# End Source File
# Begin Source File

SOURCE=.\SslCertificateAuthority.cpp
# End Source File
# Begin Source File

SOURCE=.\SslCertificateDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\SslConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\SslConfigErrorReporter.cpp
# End Source File
# Begin Source File

SOURCE=.\SslConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\SslConnectionOpenSSL.cpp
# End Source File
# Begin Source File

SOURCE=.\SslConnectionOpenSSLClient.cpp
# End Source File
# Begin Source File

SOURCE=.\SslConnectionOpenSSLServer.cpp
# End Source File
# Begin Source File

SOURCE=.\SslConnector.cpp
# End Source File
# Begin Source File

SOURCE=.\SslFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\SslGeneralConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\SslSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\SslSystemOpenSSL.cpp
# End Source File
# Begin Source File

SOURCE=.\SslTempCerts.cpp
# End Source File
# Begin Source File

SOURCE=.\SslTransceiver.cpp
# End Source File
# Begin Source File

SOURCE=.\Stream.cpp
# End Source File
# Begin Source File

SOURCE=.\SUdpClient.cpp
# End Source File
# Begin Source File

SOURCE=.\SUdpClientControlChannel.cpp
# End Source File
# Begin Source File

SOURCE=.\SUdpControlChannel.cpp
# End Source File
# Begin Source File

SOURCE=.\SUdpServerControlChannel.cpp
# End Source File
# Begin Source File

SOURCE=.\SUdpTransceiver.cpp
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

SOURCE=.\Acceptor.h
# End Source File
# Begin Source File

SOURCE=.\AcceptorF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Application.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\BasicStream.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Buffer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Communicator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\CommunicatorF.h
# End Source File
# Begin Source File

SOURCE=.\CommunicatorI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Config.h
# End Source File
# Begin Source File

SOURCE=.\Connection.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ConnectionF.h
# End Source File
# Begin Source File

SOURCE=.\ConnectionFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ConnectionFactoryF.h
# End Source File
# Begin Source File

SOURCE=.\Connector.h
# End Source File
# Begin Source File

SOURCE=.\ConnectorF.h
# End Source File
# Begin Source File

SOURCE=.\CryptKey.h
# End Source File
# Begin Source File

SOURCE=.\CryptKeyF.h
# End Source File
# Begin Source File

SOURCE=.\Cryptor.h
# End Source File
# Begin Source File

SOURCE=.\CryptorF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Current.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Direct.h
# End Source File
# Begin Source File

SOURCE=.\Endpoint.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\EndpointF.h
# End Source File
# Begin Source File

SOURCE=.\EventHandler.h
# End Source File
# Begin Source File

SOURCE=.\EventHandlerF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Exception.h
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

SOURCE=..\..\include\Ice\Identity.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\IdentityUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Incoming.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Initialize.h
# End Source File
# Begin Source File

SOURCE=.\Instance.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\InstanceF.h
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

SOURCE=..\..\include\Ice\LoggerUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\LoggerF.h
# End Source File
# Begin Source File

SOURCE=.\LoggerI.h
# End Source File
# Begin Source File

SOURCE=.\MessageAuthenticator.h
# End Source File
# Begin Source File

SOURCE=.\MessageAuthenticatorF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Native.h
# End Source File
# Begin Source File

SOURCE=.\Network.h
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

SOURCE=.\ObjectAdapterFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ObjectAdapterFactoryF.h
# End Source File
# Begin Source File

SOURCE=.\ObjectAdapterI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ObjectF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ObjectFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ObjectFactoryF.h
# End Source File
# Begin Source File

SOURCE=.\ObjectFactoryManager.h
# End Source File
# Begin Source File

SOURCE=.\ObjectFactoryManagerF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Outgoing.h
# End Source File
# Begin Source File

SOURCE=.\PicklerI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Properties.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\PropertiesF.h
# End Source File
# Begin Source File

SOURCE=.\PropertiesI.h
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

SOURCE=.\ProxyFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ProxyFactoryF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ProxyHandle.h
# End Source File
# Begin Source File

SOURCE=.\Reference.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ReferenceF.h
# End Source File
# Begin Source File

SOURCE=.\ReferenceFactory.h
# End Source File
# Begin Source File

SOURCE=.\ReferenceFactoryF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Router.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\RouterF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\RouterInfo.h
# End Source File
# Begin Source File

SOURCE=.\RouterInfo.h
# End Source File
# Begin Source File

SOURCE=.\RouterInfoF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\RoutingTable.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\RoutingTableF.h
# End Source File
# Begin Source File

SOURCE=.\SecureUdp.h
# End Source File
# Begin Source File

SOURCE=.\Security.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ServantLocator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\ServantLocatorF.h
# End Source File
# Begin Source File

SOURCE=.\SslAcceptor.h
# End Source File
# Begin Source File

SOURCE=.\SslBaseCerts.h
# End Source File
# Begin Source File

SOURCE=.\SslCertificateAuthority.h
# End Source File
# Begin Source File

SOURCE=.\SslCertificateDesc.h
# End Source File
# Begin Source File

SOURCE=.\SslConfig.h
# End Source File
# Begin Source File

SOURCE=.\SslConfigErrorReporter.h
# End Source File
# Begin Source File

SOURCE=.\SslConnection.h
# End Source File
# Begin Source File

SOURCE=.\SslConnectionOpenSSL.h
# End Source File
# Begin Source File

SOURCE=.\SslConnectionOpenSSLClient.h
# End Source File
# Begin Source File

SOURCE=.\SslConnectionOpenSSLServer.h
# End Source File
# Begin Source File

SOURCE=.\SslConnector.h
# End Source File
# Begin Source File

SOURCE=.\SslFactory.h
# End Source File
# Begin Source File

SOURCE=.\SslGeneralConfig.h
# End Source File
# Begin Source File

SOURCE=.\SslSystem.h
# End Source File
# Begin Source File

SOURCE=.\SslSystemOpenSSL.h
# End Source File
# Begin Source File

SOURCE=.\SslTempCerts.h
# End Source File
# Begin Source File

SOURCE=.\SslTransceiver.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\Stream.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\StreamF.h
# End Source File
# Begin Source File

SOURCE=.\StreamI.h
# End Source File
# Begin Source File

SOURCE=.\SUdpClient.h
# End Source File
# Begin Source File

SOURCE=.\SUdpClientControlChannel.h
# End Source File
# Begin Source File

SOURCE=.\SUdpClientF.h
# End Source File
# Begin Source File

SOURCE=.\SUdpControlChannel.h
# End Source File
# Begin Source File

SOURCE=.\SUdpControlChannelF.h
# End Source File
# Begin Source File

SOURCE=.\SUdpServerControlChannel.h
# End Source File
# Begin Source File

SOURCE=.\SUdpTransceiver.h
# End Source File
# Begin Source File

SOURCE=.\SUdpTransceiverF.h
# End Source File
# Begin Source File

SOURCE=.\TcpAcceptor.h
# End Source File
# Begin Source File

SOURCE=.\TcpConnector.h
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

SOURCE=.\UdpTransceiver.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\UserExceptionFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Ice\UserExceptionFactoryF.h
# End Source File
# Begin Source File

SOURCE=.\UserExceptionFactoryManager.h
# End Source File
# Begin Source File

SOURCE=.\UserExceptionFactoryManagerF.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\slice\Ice\BuiltinSequences.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\Ice\BuiltinSequences.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/BuiltinSequences.ice \
	move BuiltinSequences.h. ..\..\include\Ice \
	

"..\..\include\Ice\BuiltinSequences.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"BuiltinSequences.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\Ice\BuiltinSequences.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/BuiltinSequences.ice \
	move BuiltinSequences.h. ..\..\include\Ice \
	

"..\..\include\Ice\BuiltinSequences.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"BuiltinSequences.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Communicator.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

USERDEP__COMMU="../../bin/slice2cpp.exe"	
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

USERDEP__COMMU="../../bin/slice2cpp.exe"	
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

USERDEP__COMMUN="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\CommunicatorF.ice

"..\..\include\Ice\CommunicatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/CommunicatorF.ice 
	move CommunicatorF.h ..\..\include\Ice 
	del CommunicatorF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

USERDEP__COMMUN="../../bin/slice2cpp.exe"	
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

SOURCE=..\..\slice\Ice\Current.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

USERDEP__CURRE="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Current.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Current.ice \
	move Current.h ..\..\include\Ice \
	

"..\..\include\Ice\Current.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Current.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

USERDEP__CURRE="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Current.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Current.ice \
	move Current.h ..\..\include\Ice \
	

"..\..\include\Ice\Current.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Current.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Identity.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

USERDEP__IDENT="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Identity.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Identity.ice \
	move Identity.h ..\..\include\Ice \
	

"..\..\include\Ice\Identity.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Identity.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

USERDEP__IDENT="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Identity.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Identity.ice \
	move Identity.h ..\..\include\Ice \
	

"..\..\include\Ice\Identity.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Identity.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\LocalException.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

USERDEP__LOCAL="../../bin/slice2cpp.exe"	
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

USERDEP__LOCAL="../../bin/slice2cpp.exe"	
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

SOURCE=..\..\slice\Ice\Logger.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

USERDEP__LOGGE="../../bin/slice2cpp.exe"	
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

USERDEP__LOGGE="../../bin/slice2cpp.exe"	
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

USERDEP__LOGGER="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\LoggerF.ice

"..\..\include\Ice\LoggerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/LoggerF.ice 
	move LoggerF.h ..\..\include\Ice 
	del LoggerF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

USERDEP__LOGGER="../../bin/slice2cpp.exe"	
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

SOURCE=..\..\slice\Ice\ObjectAdapter.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

USERDEP__OBJEC="../../bin/slice2cpp.exe"	
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

USERDEP__OBJEC="../../bin/slice2cpp.exe"	
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

USERDEP__OBJECT="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ObjectAdapterF.ice

"..\..\include\Ice\ObjectAdapterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ObjectAdapterF.ice 
	move ObjectAdapterF.h ..\..\include\Ice 
	del ObjectAdapterF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

USERDEP__OBJECT="../../bin/slice2cpp.exe"	
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

SOURCE=..\..\slice\Ice\ObjectFactory.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

USERDEP__OBJECTF="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ObjectFactory.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ObjectFactory.ice \
	move ObjectFactory.h ..\..\include\Ice \
	

"..\..\include\Ice\ObjectFactory.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ObjectFactory.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

USERDEP__OBJECTF="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ObjectFactory.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ObjectFactory.ice \
	move ObjectFactory.h ..\..\include\Ice \
	

"..\..\include\Ice\ObjectFactory.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ObjectFactory.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\ObjectFactoryF.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

USERDEP__OBJECTFA="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ObjectFactoryF.ice

"..\..\include\Ice\ObjectFactoryF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ObjectFactoryF.ice 
	move ObjectFactoryF.h ..\..\include\Ice 
	del ObjectFactoryF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

USERDEP__OBJECTFA="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ObjectFactoryF.ice

"..\..\include\Ice\ObjectFactoryF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ObjectFactoryF.ice 
	move ObjectFactoryF.h ..\..\include\Ice 
	del ObjectFactoryF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Properties.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

USERDEP__PROPE="../../bin/slice2cpp.exe"	
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

USERDEP__PROPE="../../bin/slice2cpp.exe"	
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

USERDEP__PROPER="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\PropertiesF.ice

"..\..\include\Ice\PropertiesF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/PropertiesF.ice 
	move PropertiesF.h ..\..\include\Ice 
	del PropertiesF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

USERDEP__PROPER="../../bin/slice2cpp.exe"	
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

SOURCE=..\..\slice\Ice\Router.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

USERDEP__ROUTE="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Router.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Router.ice \
	move Router.h ..\..\include\Ice \
	

"..\..\include\Ice\Router.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Router.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

USERDEP__ROUTE="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\Router.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/Router.ice \
	move Router.h ..\..\include\Ice \
	

"..\..\include\Ice\Router.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Router.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\RouterF.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

USERDEP__ROUTER="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\RouterF.ice

"..\..\include\Ice\RouterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/RouterF.ice 
	move RouterF.h ..\..\include\Ice 
	del RouterF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

USERDEP__ROUTER="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\RouterF.ice

"..\..\include\Ice\RouterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/RouterF.ice 
	move RouterF.h ..\..\include\Ice 
	del RouterF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\SecureUdp.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

USERDEP__SECUR="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\SecureUdp.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --include-dir Ice -I../../slice ../../slice/Ice/SecureUdp.ice \
	

"SecureUdp.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"SecureUdp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

USERDEP__SECUR="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\SecureUdp.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --include-dir Ice -I../../slice ../../slice/Ice/SecureUdp.ice \
	

"SecureUdp.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"SecureUdp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\SecureUdpF.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

USERDEP__SECURE="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\SecureUdpF.ice

"SecureUdpF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --include-dir Ice -I../../slice ../../slice/Ice/SecureUdpF.ice 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

USERDEP__SECURE="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\SecureUdpF.ice

"SecureUdpF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --include-dir Ice -I../../slice ../../slice/Ice/SecureUdpF.ice 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\SecurityException.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\Ice\SecurityException.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/SecurityException.ice \
	move SecurityException.h ..\..\include\Ice \
	

"..\..\include\Ice\SecurityException.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"SecurityException.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

USERDEP__SECURI="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\SecurityException.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/SecurityException.ice \
	move SecurityException.h ..\..\include\Ice \
	

"..\..\include\Ice\SecurityException.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"SecurityException.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\ServantLocator.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

USERDEP__SERVA="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ServantLocator.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ServantLocator.ice \
	move ServantLocator.h ..\..\include\Ice \
	

"..\..\include\Ice\ServantLocator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ServantLocator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

USERDEP__SERVA="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ServantLocator.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ServantLocator.ice \
	move ServantLocator.h ..\..\include\Ice \
	

"..\..\include\Ice\ServantLocator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ServantLocator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\ServantLocatorF.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

USERDEP__SERVAN="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ServantLocatorF.ice

"..\..\include\Ice\ServantLocatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ServantLocatorF.ice 
	move ServantLocatorF.h ..\..\include\Ice 
	del ServantLocatorF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

USERDEP__SERVAN="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\ServantLocatorF.ice

"..\..\include\Ice\ServantLocatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/ServantLocatorF.ice 
	move ServantLocatorF.h ..\..\include\Ice 
	del ServantLocatorF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\Stream.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

USERDEP__STREA="../../bin/slice2cpp.exe"	
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

USERDEP__STREA="../../bin/slice2cpp.exe"	
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

USERDEP__STREAM="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\StreamF.ice

"..\..\include\Ice\StreamF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/StreamF.ice 
	move StreamF.h ..\..\include\Ice 
	del StreamF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

USERDEP__STREAM="../../bin/slice2cpp.exe"	
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

SOURCE=..\..\slice\Ice\UserExceptionFactory.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

USERDEP__USERE="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\UserExceptionFactory.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/UserExceptionFactory.ice \
	move UserExceptionFactory.h ..\..\include\Ice \
	

"..\..\include\Ice\UserExceptionFactory.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"UserExceptionFactory.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

USERDEP__USERE="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\UserExceptionFactory.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/UserExceptionFactory.ice \
	move UserExceptionFactory.h ..\..\include\Ice \
	

"..\..\include\Ice\UserExceptionFactory.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"UserExceptionFactory.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\Ice\UserExceptionFactoryF.ice

!IF  "$(CFG)" == "Ice - Win32 Release"

USERDEP__USEREX="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\UserExceptionFactoryF.ice

"..\..\include\Ice\UserExceptionFactoryF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/UserExceptionFactoryF.ice 
	move UserExceptionFactoryF.h ..\..\include\Ice 
	del UserExceptionFactoryF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Ice - Win32 Debug"

USERDEP__USEREX="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\Ice\UserExceptionFactoryF.ice

"..\..\include\Ice\UserExceptionFactoryF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir Ice -I../../slice ../../slice/Ice/UserExceptionFactoryF.ice 
	move UserExceptionFactoryF.h ..\..\include\Ice 
	del UserExceptionFactoryF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
