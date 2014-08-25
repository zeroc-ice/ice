# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# LICENSE file included in this distribution.
#
# **********************************************************************

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#

prefix			= C:\Ice-$(VERSION)

#
# Define DEBUG as yes if you want to build with debug information and
# assertions enabled.
#

DEBUG			= yes

#
# Define OPTIMIZE as yes if you want to build with optimization.
#

OPTIMIZE		= yes

#
# Define VS to VS2013 or VS2012 to build with that Visual Studio 
# Version.
#
#VS             = VS2012
 
#
# Set the key file used to sign assemblies.
#

!if "$(KEYFILE)" == ""
KEYFILE			= $(top_srcdir)\config\IceDevKey.snk
!endif


# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Common definitions
#

ice_language     = cs
!if exist ($(top_srcdir)\..\config\Make.common.rules.mak)
!include $(top_srcdir)\..\config\Make.common.rules.mak
!else
!include $(top_srcdir)\config\Make.common.rules.mak
!endif

EVERYTHING      = all install clean

#
# Registry keywords required by Visual Studio and Ice Visual Studio Add-in.
#
!if "$(PROCESSOR_ARCHITECTURE)" == "AMD64"
REGISTRY_PREFIX				= HKLM\Software\Wow6432Node
!else
REGISTRY_PREFIX				= HKLM\Software
!endif

INSTALL_KEY					= $(REGISTRY_PREFIX)\ZeroC\Ice $(VERSION)
DOTNET_ASSEMBLIES_KEY		= $(REGISTRY_PREFIX)\Microsoft\.NETFramework\v2.0.50727\AssemblyFoldersEx\Ice
POCKETPC_ASSEMBLIES_KEY		= $(REGISTRY_PREFIX)\Microsoft\.NETCompactFramework\v3.5.0.0\PocketPC\AssemblyFoldersEx\Ice
SMARTPHONE_ASSEMBLIES_KEY	= $(REGISTRY_PREFIX)\Microsoft\.NETCompactFramework\v3.5.0.0\Smartphone\AssemblyFoldersEx\Ice
WINDOWSCE_ASSEMBLIES_KEY	= $(REGISTRY_PREFIX)\Microsoft\.NETCompactFramework\v3.5.0.0\WindowsCE\AssemblyFoldersEx\Ice
SILVERLIGH_ASSEMBLIES_KEY	= $(REGISTRY_PREFIX)\Microsoft SDKs\Silverlight\v5.0\AssemblyFoldersEx\Ice

#
# Visual Studio version
#

!if "$(VS)" == "" && "$(VSSDK120INSTALL)" != ""
VS = VS2013
!message VS2013 will be used!
!elseif "$(VS)" == "" && "$(VSSDK110INSTALL)" != ""
VS = VS2012
!message VS2012 will be used!
!endif

!if "$(VS)" == "VS2013" 
VSSDK_HOME              = $(VSSDK120INSTALL)
ADDIN_PREFIX            = $(ALLUSERSPROFILE)\Microsoft\VisualStudio\12.0\Addins
!elseif "$(VS)" == "VS2012" 
VSSDK_HOME   		= $(VSSDK110INSTALL)
ADDIN_PREFIX 		= $(ALLUSERSPROFILE)\Microsoft\VisualStudio\11.0\Addins
!endif

!if "$(VSSDK_HOME)" == ""
!message The Visual Studio SDK does not appear to be installed: VSSDK120INSTALL and VSSDK110INSTALL are both unset.
!endif

VSTARGET                = $(VS)
VS_HOME 		= $(VSINSTALLDIR)
PKG_PREFIX 		= $(VSTARGET)
PROPERTY_SHEET = "Ice.props"

bindir 			= ..\bin

install_bindir 		= $(prefix)\vsaddin
install_configdir 	= $(prefix)\config

OBJEXT			= .obj

MCS			= csc -nologo

MCSFLAGS 		= -d:MAKEFILE_BUILD -target:library -keyfile:$(KEYFILE) -warnaserror+

#
# Supress EnvDTE redirection warning.
#
MCSFLAGS 		= $(MCSFLAGS) -nowarn:1701

!if "$(DEBUG)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -debug -define:DEBUG
!endif

!if "$(OPTIMIZE)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -optimize+
!endif

MCSFLAGS = $(MCSFLAGS) -define:$(VSTARGET)


MCSFLAGS = $(MCSFLAGS) /reference:"$(PROGRAMFILES)\Common Files\microsoft shared\MSEnv\PublicAssemblies\EnvDTE.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(PROGRAMFILES)\Common Files\microsoft shared\MSEnv\PublicAssemblies\EnvDTE80.dll"

MCSFLAGS = $(MCSFLAGS) /reference:"$(PROGRAMFILES)\Common Files\microsoft shared\MSEnv\PublicAssemblies\Microsoft.VisualStudio.CommandBars.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(PROGRAMFILES)\Common Files\microsoft shared\MSEnv\PublicAssemblies\VSLangProj.dll"

MCSFLAGS = $(MCSFLAGS) /reference:"$(VS_HOME)\Common7\IDE\PublicAssemblies\Microsoft.VisualStudio.VCProject.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VS_HOME)\Common7\IDE\PublicAssemblies\Microsoft.VisualStudio.VCProjectEngine.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"Microsoft.Build.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(PROGRAMFILES)\Common Files\microsoft shared\MSEnv\PublicAssemblies\Extensibility.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\v2.0\Microsoft.VisualStudio.OLE.Interop.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VS_HOME)\Common7\IDE\PrivateAssemblies\Microsoft.VisualStudio.Shell.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\v2.0\Microsoft.VisualStudio.Shell.Interop.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\v2.0\Microsoft.VisualStudio.Shell.Interop.8.0.dll"

all::
