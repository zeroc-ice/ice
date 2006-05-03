# Microsoft Developer Studio Project File - Name="ice" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ice - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ice.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ice.mak" CFG="ice - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ice - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ice - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ice - Win32 Release Static" (based on "Win32 (x86) Static Library")
!MESSAGE "ice - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

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
SLICE2CPPEFLAGS=-DICEE
CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBRARY_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O1 /I ".." /I "../../include" /D "NDEBUG" /D "_USRDLL" /D "ICE_API_EXPORTS" /D "WIN32_LEAN_AND_MEAN" /D FD_SETSIZE=1024 /D "_CONSOLE" /FD /c
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
# ADD LINK32 ws2_32.lib rpcrt4.lib /nologo /dll /incremental:yes /debug /machine:I386 /out:"Release/icee11.dll" /implib:"Release/icee.lib"
# SUBTRACT LINK32 /pdb:none /nodefaultlib
# Begin Special Build Tool
OutDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\icee.lib ..\..\lib\	copy $(OutDir)\icee11.dll ..\..\bin	copy $(OutDir)\icee11.pdb ..\..\bin
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
SLICE2CPPEFLAGS=-DICEE
CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBRARY_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "_DEBUG" /D "_USRDLL" /D "ICE_API_EXPORTS" /D "WIN32_LEAN_AND_MEAN" /D FD_SETSIZE=1024 /D "_CONSOLE" /FD /GZ /c
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
# ADD LINK32 ws2_32.lib rpcrt4.lib /nologo /dll /debug /machine:I386 /out:"Debug/icee11d.dll" /implib:"Debug/iceed.lib"
# SUBTRACT LINK32 /profile /pdb:none /incremental:no /nodefaultlib
# Begin Special Build Tool
OutDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\iceed.lib ..\..\lib\	copy $(OutDir)\icee11d.pdb ..\..\bin	copy $(OutDir)\icee11d.dll ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ice - Win32 Release Static"

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
MTL=midl.exe
SLICE2CPPEFLAGS=-DICEE
CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O1 /I ".." /I "../../include" /D "NDEBUG" /D "ICEE_STATIC_LIBS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /D FD_SETSIZE=1024 /D "_CONSOLE" /FD /c
# SUBTRACT CPP /Fr /YX
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD LIB32 /nologo /out:"ReleaseStatic\icee.lib"
# Begin Special Build Tool
OutDir=.\ReleaseStatic
SOURCE="$(InputPath)"
PostBuild_Cmds=copy                                          $(OutDir)\icee.lib                                          ..\..\lib\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ice - Win32 Debug Static"

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
MTL=midl.exe
SLICE2CPPEFLAGS=-DICEE
CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "_DEBUG" /D "ICEE_STATIC_LIBS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /D FD_SETSIZE=1024 /D "_CONSOLE" /FD /GZ /c
# SUBTRACT CPP /Fr /YX
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"DebugStatic/icee.bsc"
LIB32=link.exe -lib
# ADD LIB32 /nologo /out:"DebugStatic\iceed.lib"
# Begin Special Build Tool
OutDir=.\DebugStatic
SOURCE="$(InputPath)"
PostBuild_Cmds=copy                                         $(OutDir)\iceed.lib                                         ..\..\lib\ 
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "ice - Win32 Release"
# Name "ice - Win32 Debug"
# Name "ice - Win32 Release Static"
# Name "ice - Win32 Debug Static"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\TcpTransport\Acceptor.cpp
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

SOURCE=.\Cond.cpp
# End Source File
# Begin Source File

SOURCE=.\Connection.cpp
# End Source File
# Begin Source File

SOURCE=..\TcpTransport\Connector.cpp
# End Source File
# Begin Source File

SOURCE=.\ConvertUTF.cpp
# End Source File
# Begin Source File

SOURCE=.\Current.cpp
# End Source File
# Begin Source File

SOURCE=.\DefaultsAndOverrides.cpp
# End Source File
# Begin Source File

SOURCE=.\Endpoint.cpp
# End Source File
# Begin Source File

SOURCE=..\TcpTransport\EndpointFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\ExceptionBase.cpp
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

SOURCE=.\IncomingConnectionFactory.cpp
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

SOURCE=.\OperationMode.cpp
# End Source File
# Begin Source File

SOURCE=.\Outgoing.cpp
# End Source File
# Begin Source File

SOURCE=.\OutgoingConnectionFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\Properties.cpp
# End Source File
# Begin Source File

SOURCE=.\Protocol.cpp
# End Source File
# Begin Source File

SOURCE=.\Proxy.cpp
# End Source File
# Begin Source File

SOURCE=.\ProxyFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\RecMutex.cpp
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

SOURCE=.\RWRecMutex.cpp
# End Source File
# Begin Source File

SOURCE=.\SafeStdio.cpp
# End Source File
# Begin Source File

SOURCE=.\ServantManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared.cpp
# End Source File
# Begin Source File

SOURCE=.\StaticMutex.cpp
# End Source File
# Begin Source File

SOURCE=.\StringConverter.cpp
# End Source File
# Begin Source File

SOURCE=.\StringUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\TcpTransport\TcpEndpoint.cpp
# End Source File
# Begin Source File

SOURCE=.\Thread.cpp
# End Source File
# Begin Source File

SOURCE=.\ThreadException.cpp
# End Source File
# Begin Source File

SOURCE=.\Time.cpp
# End Source File
# Begin Source File

SOURCE=.\TraceLevels.cpp
# End Source File
# Begin Source File

SOURCE=.\TraceUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\TcpTransport\Transceiver.cpp
# End Source File
# Begin Source File

SOURCE=.\Unicode.cpp
# End Source File
# Begin Source File

SOURCE=.\UnknownEndpoint.cpp
# End Source File
# Begin Source File

SOURCE=.\UUID.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\IceE\AbstractMutex.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Algorithm.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Buffer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\BuiltinSequences.h
# End Source File
# Begin Source File

SOURCE=.\ConvertUTF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Current.h
# End Source File
# Begin Source File

SOURCE=.\DefaultsAndOverrides.h
# End Source File
# Begin Source File

SOURCE=.\DefaultsAndOverridesF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\DispatchStatus.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\ExceptionBase.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\FacetMap.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\IceE.h
# End Source File
# Begin Source File

SOURCE=.\IncomingConnectionFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\IncomingConnectionFactoryF.h
# End Source File
# Begin Source File

SOURCE=.\Instance.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Iterator.h
# End Source File
# Begin Source File

SOURCE=.\LocatorInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\LocatorInfoF.h
# End Source File
# Begin Source File

SOURCE=.\LocatorInfoF.h
# End Source File
# Begin Source File

SOURCE=.\LoggerI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Monitor.h
# End Source File
# Begin Source File

SOURCE=.\Network.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Object.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\ObjectAdapter.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\ObjectAdapterF.h
# End Source File
# Begin Source File

SOURCE=.\ObjectAdapterFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\ObjectAdapterFactoryF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\ObjectF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\OperationMode.h
# End Source File
# Begin Source File

SOURCE=.\OutgoingConnectionFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\OutgoingConnectionFactoryF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Protocol.h
# End Source File
# Begin Source File

SOURCE=.\ProxyFactory.h
# End Source File
# Begin Source File

SOURCE=.\Reference.h
# End Source File
# Begin Source File

SOURCE=.\ReferenceFactory.h
# End Source File
# Begin Source File

SOURCE=.\ReferenceFactoryF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Router.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\RouterF.h
# End Source File
# Begin Source File

SOURCE=.\RouterInfo.h
# End Source File
# Begin Source File

SOURCE=.\RouterInfoF.h
# End Source File
# Begin Source File

SOURCE=.\ServantManager.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Time.h
# End Source File
# Begin Source File

SOURCE=.\TraceLevels.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\TraceLevelsF.h
# End Source File
# Begin Source File

SOURCE=.\TraceUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\UndefSysMacros.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\Unicode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceE\UUID.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\slice\IceE\BuiltinSequences.ice

!IF  "$(CFG)" == "ice - Win32 Release"

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

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

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

!ELSEIF  "$(CFG)" == "ice - Win32 Release Static"

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

!ELSEIF  "$(CFG)" == "ice - Win32 Debug Static"

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

!IF  "$(CFG)" == "ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceE\FacetMap.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/FacetMap.ice \
	move FacetMap.h ..\..\include\icee \
	

"..\..\include\icee\FacetMap.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"FacetMap.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceE\FacetMap.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/FacetMap.ice \
	move FacetMap.h ..\..\include\icee \
	

"..\..\include\icee\FacetMap.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"FacetMap.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Release Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\FacetMap.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/FacetMap.ice \
	move FacetMap.h ..\..\include\icee \
	

"..\..\include\icee\FacetMap.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"FacetMap.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\FacetMap.ice

BuildCmds= \
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/FacetMap.ice \
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

!IF  "$(CFG)" == "ice - Win32 Release"

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

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

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

!ELSEIF  "$(CFG)" == "ice - Win32 Release Static"

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

!ELSEIF  "$(CFG)" == "ice - Win32 Debug Static"

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

!IF  "$(CFG)" == "ice - Win32 Release"

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

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

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

!ELSEIF  "$(CFG)" == "ice - Win32 Release Static"

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

!ELSEIF  "$(CFG)" == "ice - Win32 Debug Static"

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

!IF  "$(CFG)" == "ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceE\LocatorF.ice

"..\..\include\icee\LocatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/LocatorF.ice 
	move LocatorF.h ..\..\include\icee 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceE\LocatorF.ice

"..\..\include\icee\LocatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/LocatorF.ice 
	move LocatorF.h ..\..\include\icee 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Release Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\LocatorF.ice

"..\..\include\icee\LocatorF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/LocatorF.ice 
	move LocatorF.h ..\..\include\icee 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug Static"

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

!IF  "$(CFG)" == "ice - Win32 Release"

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

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

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

!ELSEIF  "$(CFG)" == "ice - Win32 Release Static"

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

!ELSEIF  "$(CFG)" == "ice - Win32 Debug Static"

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

!IF  "$(CFG)" == "ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceE\LoggerF.ice

"..\..\include\icee\LoggerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/LoggerF.ice 
	move LoggerF.h ..\..\include\icee 
	del LoggerF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceE\LoggerF.ice

"..\..\include\icee\LoggerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/LoggerF.ice 
	move LoggerF.h ..\..\include\icee 
	del LoggerF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Release Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\LoggerF.ice

"..\..\include\icee\LoggerF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/LoggerF.ice 
	move LoggerF.h ..\..\include\icee 
	del LoggerF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug Static"

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

!IF  "$(CFG)" == "ice - Win32 Release"

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

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

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

!ELSEIF  "$(CFG)" == "ice - Win32 Release Static"

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

!ELSEIF  "$(CFG)" == "ice - Win32 Debug Static"

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

!IF  "$(CFG)" == "ice - Win32 Release"

# Begin Custom Build
InputPath=..\..\slice\IceE\RouterF.ice

"..\..\include\icee\RouterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/RouterF.ice 
	move RouterF.h ..\..\include\icee 
	del RouterF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug"

# Begin Custom Build
InputPath=..\..\slice\IceE\RouterF.ice

"..\..\include\icee\RouterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/RouterF.ice 
	move RouterF.h ..\..\include\icee 
	del RouterF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Release Static"

# Begin Custom Build
InputPath=..\..\slice\IceE\RouterF.ice

"..\..\include\icee\RouterF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	slice2cppe.exe --ice --dll-export ICE_API --include-dir IceE -I../../slice ../../slice/IceE/RouterF.ice 
	move RouterF.h ..\..\include\icee 
	del RouterF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ice - Win32 Debug Static"

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
