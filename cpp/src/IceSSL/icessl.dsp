# Microsoft Developer Studio Project File - Name="IceSSL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=IceSSL - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "IceSSL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "IceSSL.mak" CFG="IceSSL - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "IceSSL - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "IceSSL - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "IceSSL - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /WX /GR /GX /O2 /I ".." /I "../../include" /D "_USRDLL" /D "ICE_SSL_API_EXPORTS" /D "USE_SOCKETS" /D "NDEBUG" /D "_CONSOLE" /D "_UNICODE" /YX /FD /c
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
# ADD LINK32 ws2_32.lib libeay32.lib ssleay32.lib xerces-c_2.lib /nologo /dll /machine:I386 /out:"Release/icessl11.dll" /implib:"Release/icessl.lib"
# SUBTRACT LINK32 /pdb:none /debug /nodefaultlib
# Begin Special Build Tool
OutDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\icessl.lib ..\..\lib	copy $(OutDir)\icessl11.dll ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "IceSSL - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /WX /Gm /GR /GX /Zi /Od /I ".." /I "../../include" /D "USE_SOCKETS" /D "_USRDLL" /D "ICE_SSL_API_EXPORTS" /D "_DEBUG" /D "_CONSOLE" /D "_UNICODE" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386
# ADD LINK32 ws2_32.lib libeay32.lib ssleay32.lib xerces-c_2D.lib /nologo /dll /debug /machine:I386 /out:"Debug/icessl11d.dll" /implib:"Debug/icessld.lib"
# SUBTRACT LINK32 /pdb:none /nodefaultlib
# Begin Special Build Tool
OutDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\icessld.lib ..\..\lib	copy $(OutDir)\icessl11d.pdb ..\..\bin	copy $(OutDir)\icessl11d.dll ..\..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "IceSSL - Win32 Release"
# Name "IceSSL - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BaseCerts.cpp
# End Source File
# Begin Source File

SOURCE=.\CertificateAuthority.cpp
# End Source File
# Begin Source File

SOURCE=.\CertificateDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\CertificateVerifier.cpp
# End Source File
# Begin Source File

SOURCE=.\CertificateVerifierOpenSSL.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientContext.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigParser.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigParserErrorReporter.cpp
# End Source File
# Begin Source File

SOURCE=.\Context.cpp
# End Source File
# Begin Source File

SOURCE=.\Convert.cpp
# End Source File
# Begin Source File

SOURCE=.\DefaultCertificateVerifier.cpp
# End Source File
# Begin Source File

SOURCE=.\DHParams.cpp
# End Source File
# Begin Source File

SOURCE=.\Exception.cpp
# End Source File
# Begin Source File

SOURCE=.\GeneralConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\OpenSSLJanitors.cpp
# End Source File
# Begin Source File

SOURCE=.\OpenSSLPluginI.cpp
# End Source File
# Begin Source File

SOURCE=.\OpenSSLUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\Plugin.cpp
# End Source File
# Begin Source File

SOURCE=.\RSACertificateGen.cpp
# End Source File
# Begin Source File

SOURCE=.\RSAKeyPair.cpp
# End Source File
# Begin Source File

SOURCE=.\RSAPrivateKey.cpp
# End Source File
# Begin Source File

SOURCE=.\RSAPublicKey.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerContext.cpp
# End Source File
# Begin Source File

SOURCE=.\SingleCertificateVerifier.cpp
# End Source File
# Begin Source File

SOURCE=.\SslAcceptor.cpp
# End Source File
# Begin Source File

SOURCE=.\SslClientTransceiver.cpp
# End Source File
# Begin Source File

SOURCE=.\SslConnector.cpp
# End Source File
# Begin Source File

SOURCE=.\SslEndpoint.cpp
# End Source File
# Begin Source File

SOURCE=.\SslException.cpp
# End Source File
# Begin Source File

SOURCE=.\SslServerTransceiver.cpp
# End Source File
# Begin Source File

SOURCE=.\SslTransceiver.cpp
# End Source File
# Begin Source File

SOURCE=.\TempCerts.cpp
# End Source File
# Begin Source File

SOURCE=.\TraceLevels.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BaseCerts.h
# End Source File
# Begin Source File

SOURCE=.\CertificateAuthority.h
# End Source File
# Begin Source File

SOURCE=.\CertificateDesc.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceSSL\CertificateVerifier.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceSSL\CertificateVerifierF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceSSL\CertificateVerifierOpenSSL.h
# End Source File
# Begin Source File

SOURCE=.\ClientContext.h
# End Source File
# Begin Source File

SOURCE=.\ConfigParser.h
# End Source File
# Begin Source File

SOURCE=.\ConfigParserErrorReporter.h
# End Source File
# Begin Source File

SOURCE=.\Context.h
# End Source File
# Begin Source File

SOURCE=.\ContextF.h
# End Source File
# Begin Source File

SOURCE=.\Convert.h
# End Source File
# Begin Source File

SOURCE=.\DefaultCertificateVerifier.h
# End Source File
# Begin Source File

SOURCE=.\DHParams.h
# End Source File
# Begin Source File

SOURCE=.\DHParamsF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceSSL\Exception.h
# End Source File
# Begin Source File

SOURCE=.\GeneralConfig.h
# End Source File
# Begin Source File

SOURCE=.\OpenSSL.h
# End Source File
# Begin Source File

SOURCE=.\OpenSSLJanitors.h
# End Source File
# Begin Source File

SOURCE=.\OpenSSLPluginI.h
# End Source File
# Begin Source File

SOURCE=.\OpenSSLPluginIF.h
# End Source File
# Begin Source File

SOURCE=.\OpenSSLUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceSSL\Plugin.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceSSL\PluginF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceSSL\RSACertificateGen.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceSSL\RSACertificateGenF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceSSL\RSAKeyPair.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceSSL\RSAKeyPairF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceSSL\RSAPrivateKey.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceSSL\RSAPrivateKeyF.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceSSL\RSAPublicKey.h
# End Source File
# Begin Source File

SOURCE=..\..\include\IceSSL\RSAPublicKeyF.h
# End Source File
# Begin Source File

SOURCE=.\ServerContext.h
# End Source File
# Begin Source File

SOURCE=.\SingleCertificateVerifier.h
# End Source File
# Begin Source File

SOURCE=.\SslAcceptor.h
# End Source File
# Begin Source File

SOURCE=.\SslClientTransceiver.h
# End Source File
# Begin Source File

SOURCE=.\SslConnection.h
# End Source File
# Begin Source File

SOURCE=.\SslConnectionF.h
# End Source File
# Begin Source File

SOURCE=.\SslConnector.h
# End Source File
# Begin Source File

SOURCE=.\SslEndpoint.h
# End Source File
# Begin Source File

SOURCE=.\SslServerTransceiver.h
# End Source File
# Begin Source File

SOURCE=.\SslTransceiver.h
# End Source File
# Begin Source File

SOURCE=.\SslTransceiverF.h
# End Source File
# Begin Source File

SOURCE=.\TempCerts.h
# End Source File
# Begin Source File

SOURCE=.\TraceLevels.h
# End Source File
# Begin Source File

SOURCE=.\TraceLevelsF.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\slice\IceSSL\CertificateVerifier.ice

!IF  "$(CFG)" == "IceSSL - Win32 Release"

USERDEP__CERTI="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\IceSSL\CertificateVerifier.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_SSL_API --include-dir IceSSL -I../../slice ../../slice/IceSSL/CertificateVerifier.ice \
	move CertificateVerifier.h ..\..\include\IceSSL \
	

"..\..\include\IceSSL\CertificateVerifier.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"CertificateVerifier.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IceSSL - Win32 Debug"

USERDEP__CERTI="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\IceSSL\CertificateVerifier.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_SSL_API --include-dir IceSSL -I../../slice ../../slice/IceSSL/CertificateVerifier.ice \
	move CertificateVerifier.h ..\..\include\IceSSL \
	

"..\..\include\IceSSL\CertificateVerifier.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"CertificateVerifier.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceSSL\CertificateVerifierF.ice

!IF  "$(CFG)" == "IceSSL - Win32 Release"

USERDEP__CERTIF="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\IceSSL\CertificateVerifierF.ice

"..\..\include\IceSSL\CertificateVerifierF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_SSL_API --include-dir IceSSL -I../../slice ../../slice/IceSSL/CertificateVerifierF.ice 
	move CertificateVerifierF.h ..\..\include\IceSSL 
	del CertificateVerifierF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "IceSSL - Win32 Debug"

USERDEP__CERTIF="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\IceSSL\CertificateVerifierF.ice

"..\..\include\IceSSL\CertificateVerifierF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_SSL_API --include-dir IceSSL -I../../slice ../../slice/IceSSL/CertificateVerifierF.ice 
	move CertificateVerifierF.h ..\..\include\IceSSL 
	del CertificateVerifierF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceSSL\Exception.ice

!IF  "$(CFG)" == "IceSSL - Win32 Release"

USERDEP__EXCEP="..\..\bin\slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\IceSSL\Exception.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_SSL_API --include-dir IceSSL -I../../slice ../../slice/IceSSL/Exception.ice \
	move Exception.h ..\..\include\IceSSL \
	

"..\..\include\IceSSL\Exception.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Exception.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IceSSL - Win32 Debug"

USERDEP__EXCEP="..\..\bin\slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\IceSSL\Exception.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_SSL_API --include-dir IceSSL -I../../slice ../../slice/IceSSL/Exception.ice \
	move Exception.h ..\..\include\IceSSL \
	

"..\..\include\IceSSL\Exception.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Exception.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceSSL\Plugin.ice

!IF  "$(CFG)" == "IceSSL - Win32 Release"

USERDEP__PLUGI="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\IceSSL\Plugin.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_SSL_API --include-dir IceSSL -I../../slice ../../slice/IceSSL/Plugin.ice \
	move Plugin.h ..\..\include\IceSSL \
	

"..\..\include\IceSSL\Plugin.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Plugin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IceSSL - Win32 Debug"

USERDEP__PLUGI="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\IceSSL\Plugin.ice

BuildCmds= \
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_SSL_API --include-dir IceSSL -I../../slice ../../slice/IceSSL/Plugin.ice \
	move Plugin.h ..\..\include\IceSSL \
	

"..\..\include\IceSSL\Plugin.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Plugin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\slice\IceSSL\PluginF.ice

!IF  "$(CFG)" == "IceSSL - Win32 Release"

USERDEP__PLUGIN="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\IceSSL\PluginF.ice

"..\..\include\IceSSL\PluginF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_SSL_API --include-dir IceSSL -I../../slice ../../slice/IceSSL/PluginF.ice 
	move PluginF.h ..\..\include\IceSSL 
	del PluginF.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "IceSSL - Win32 Debug"

USERDEP__PLUGIN="../../bin/slice2cpp.exe"	
# Begin Custom Build
InputPath=..\..\slice\IceSSL\PluginF.ice

"..\..\include\IceSSL\PluginF.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\bin\slice2cpp.exe --ice --dll-export ICE_SSL_API --include-dir IceSSL -I../../slice ../../slice/IceSSL/PluginF.ice 
	move PluginF.h ..\..\include\IceSSL 
	del PluginF.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
