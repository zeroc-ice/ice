# Microsoft Developer Studio Project File - Name="IcePack" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=IcePack - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "IcePack.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "IcePack.mak" CFG="IcePack - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "IcePack - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "IcePack - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "IcePack - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I ".." /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ICE_API_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 jtc.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /dll /machine:I386 /out:"Release/ice001.dll"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Release\ice001.* ..\..\lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "IcePack - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I ".." /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ICE_API_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 jtcd.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"Debug/ice001d.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Debug\ice001d.* ..\..\lib
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "IcePack - Win32 Release"
# Name "IcePack - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Acceptor.cpp
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

SOURCE=.\Pickler.cpp
# End Source File
# Begin Source File

SOURCE=.\PicklerI.cpp
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

SOURCE=.\Shared.cpp
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

SOURCE=.\ValueFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\ValueFactoryManager.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\IcePack\Acceptor.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\AcceptorF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\Buffer.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\Collector.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\CollectorF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\Communicator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\CommunicatorF.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\CommunicatorI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\Config.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\Connector.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\ConnectorF.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\Emitter.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\EmitterF.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\Endpoint.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\EndpointF.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\EventHandler.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\EventHandlerF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\Functional.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\Handle.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\IcePack.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\Incoming.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\Initialize.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\Instance.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\InstanceF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\LocalException.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\LocalObject.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\LocalObjectF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\Logger.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\LoggerF.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\LoggerI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\Native.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\Network.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\Object.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\ObjectAdapter.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\ObjectAdapterF.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\ObjectAdapterFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\ObjectAdapterFactoryF.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\ObjectAdapterI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\ObjectF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\Outgoing.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\Pickler.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\PicklerF.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\PicklerI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\Properties.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\PropertiesF.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\PropertiesI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\Proxy.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\ProxyF.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\ProxyFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\ProxyFactoryF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\ProxyHandle.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\Reference.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\ReferenceF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\Shared.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\SslAcceptor.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\SslConnector.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\SslTransceiver.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\Stream.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\TcpAcceptor.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\TcpConnector.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\TcpTransceiver.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\ThreadPool.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\ThreadPoolF.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\TraceLevels.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\TraceLevelsF.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\TraceUtil.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\Transceiver.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\TransceiverF.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\UdpTransceiver.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\ValueFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IcePack\ValueFactoryF.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\ValueFactoryManager.h
# End Source File
# Begin Source File

SOURCE=.\IcePack\ValueFactoryManagerF.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\slice\IcePack\Communicator.ice

!IF  "$(CFG)" == "IcePack - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IcePack\Communicator.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/Communicator.ice \
	move Communicator.h ..\..\include\IcePack \
	

"..\..\include\IcePack\Communicator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Communicator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IcePack - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IcePack\Communicator.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/Communicator.ice \
	move Communicator.h ..\..\include\IcePack \
	

"..\..\include\IcePack\Communicator.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Communicator.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IcePack\CommunicatorF.ice

!IF  "$(CFG)" == "IcePack - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IcePack\CommunicatorF.ice

"..\..\include\IcePack\CommunicatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/CommunicatorF.ice 
	move CommunicatorF.h ..\..\include\IcePack 
	del CommunicatorF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "IcePack - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IcePack\CommunicatorF.ice

"..\..\include\IcePack\CommunicatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/CommunicatorF.ice 
	move CommunicatorF.h ..\..\include\IcePack 
	del CommunicatorF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IcePack\Logger.ice

!IF  "$(CFG)" == "IcePack - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IcePack\Logger.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/Logger.ice \
	move Logger.h ..\..\include\IcePack \
	

"..\..\include\IcePack\Logger.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Logger.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IcePack - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IcePack\Logger.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/Logger.ice \
	move Logger.h ..\..\include\IcePack \
	

"..\..\include\IcePack\Logger.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Logger.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IcePack\LoggerF.ice

!IF  "$(CFG)" == "IcePack - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IcePack\LoggerF.ice

"..\..\include\IcePack\LoggerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/LoggerF.ice 
	move LoggerF.h ..\..\include\IcePack 
	del LoggerF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "IcePack - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IcePack\LoggerF.ice

"..\..\include\IcePack\LoggerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/LoggerF.ice 
	move LoggerF.h ..\..\include\IcePack 
	del LoggerF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IcePack\ObjectAdapter.ice

!IF  "$(CFG)" == "IcePack - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IcePack\ObjectAdapter.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/ObjectAdapter.ice \
	move ObjectAdapter.h ..\..\include\IcePack \
	

"..\..\include\IcePack\ObjectAdapter.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ObjectAdapter.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IcePack - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IcePack\ObjectAdapter.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/ObjectAdapter.ice \
	move ObjectAdapter.h ..\..\include\IcePack \
	

"..\..\include\IcePack\ObjectAdapter.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ObjectAdapter.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IcePack\ObjectAdapterF.ice

!IF  "$(CFG)" == "IcePack - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IcePack\ObjectAdapterF.ice

"..\..\include\IcePack\ObjectAdapterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/ObjectAdapterF.ice 
	move ObjectAdapterF.h ..\..\include\IcePack 
	del ObjectAdapterF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "IcePack - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IcePack\ObjectAdapterF.ice

"..\..\include\IcePack\ObjectAdapterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/ObjectAdapterF.ice 
	move ObjectAdapterF.h ..\..\include\IcePack 
	del ObjectAdapterF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IcePack\Pickler.ice

!IF  "$(CFG)" == "IcePack - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IcePack\Pickler.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/Pickler.ice \
	move Pickler.h ..\..\include\IcePack \
	

"..\..\include\IcePack\Pickler.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Pickler.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IcePack - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IcePack\Pickler.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/Pickler.ice \
	move Pickler.h ..\..\include\IcePack \
	

"..\..\include\IcePack\Pickler.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Pickler.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IcePack\PicklerF.ice

!IF  "$(CFG)" == "IcePack - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IcePack\PicklerF.ice

"..\..\include\IcePack\PicklerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/PicklerF.ice 
	move PicklerF.h ..\..\include\IcePack 
	del PicklerF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "IcePack - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IcePack\PicklerF.ice

"..\..\include\IcePack\PicklerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/PicklerF.ice 
	move PicklerF.h ..\..\include\IcePack 
	del PicklerF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IcePack\Properties.ice

!IF  "$(CFG)" == "IcePack - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IcePack\Properties.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/Properties.ice \
	move Properties.h ..\..\include\IcePack \
	

"..\..\include\IcePack\Properties.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Properties.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IcePack - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IcePack\Properties.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/Properties.ice \
	move Properties.h ..\..\include\IcePack \
	

"..\..\include\IcePack\Properties.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Properties.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IcePack\PropertiesF.ice

!IF  "$(CFG)" == "IcePack - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IcePack\PropertiesF.ice

"..\..\include\IcePack\PropertiesF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/PropertiesF.ice 
	move PropertiesF.h ..\..\include\IcePack 
	del PropertiesF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "IcePack - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IcePack\PropertiesF.ice

"..\..\include\IcePack\PropertiesF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/PropertiesF.ice 
	move PropertiesF.h ..\..\include\IcePack 
	del PropertiesF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IcePack\ValueFactory.ice

!IF  "$(CFG)" == "IcePack - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IcePack\ValueFactory.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/ValueFactory.ice \
	move ValueFactory.h ..\..\include\IcePack \
	

"..\..\include\IcePack\ValueFactory.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ValueFactory.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IcePack - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IcePack\ValueFactory.ice

BuildCmds= \
	set PATH=%PATH%;..\..\lib \
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/ValueFactory.ice \
	move ValueFactory.h ..\..\include\IcePack \
	

"..\..\include\IcePack\ValueFactory.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ValueFactory.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IcePack\ValueFactoryF.ice

!IF  "$(CFG)" == "IcePack - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IcePack\ValueFactoryF.ice

"..\..\include\IcePack\ValueFactoryF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/ValueFactoryF.ice 
	move ValueFactoryF.h ..\..\include\IcePack 
	del ValueFactoryF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "IcePack - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IcePack\ValueFactoryF.ice

"..\..\include\IcePack\ValueFactoryF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;..\..\lib 
	..\..\bin\slice2cpp.exe --dll-export ICE_API --include-dir IcePack -I../../slice ../../slice/IcePack/ValueFactoryF.ice 
	move ValueFactoryF.h ..\..\include\IcePack 
	del ValueFactoryF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
