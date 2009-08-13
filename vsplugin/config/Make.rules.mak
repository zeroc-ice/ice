# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# LICENSE file included in this distribution.
#
# **********************************************************************

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#

prefix			= C:\IceVisualStudioExtension-$(VERSION)

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
# Set the target Visual Studio Version  Supported Versions are:
#	VS2008 - Visual Studio 2008
#	VS2005 - Visual Studio 2005
#

!if "$(VS)" == ""
VS			= VS2008
!endif

#
# Set the location of the Visual Studio 2005,  This is only required if you are
# building the extension for Visual Studio 2005
#
VS2005_HOME		= C:\Program Files\Microsoft Visual Studio 8
#VS2005_HOME 		 = C:\Program Files (x86)\Microsoft Visual Studio 8

#
# Set the location of the Visual Studio 2008 SDK
#

VSSDK_HOME		= C:\Program Files\Microsoft Visual Studio 2008 SDK
#VSSDK_HOME		 = C:\Program Files (x86)\Microsoft Visual Studio 2008 SDK

#
# Set the key file used to sign assemblies.
#

!if "$(KEYFILE)" == ""
KEYFILE			= $(top_srcdir)\config\IceDevKey.snk
!endif

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

EVERYTHING      = all install clean

!if "$(VS)" == "VS2008"
VS_HOME = $(VSINSTALLDIR)
PKG_PREFIX = $(VS)
!else if "$(VS)" == "VS2005"
VS_HOME = $(VS2005_HOME)
PKG_PREFIX = $(VS)
!else
!error Invalid setting for VS: $(VS)
!endif

bindir = ..\bin

install_bindir = $(prefix)\bin
install_configdir 	= $(prefix)\config

VERSION = 1.0.0
INTVERSION		= 1.0.0
SHORT_VERSION           = 1.0
SOVERSION		= 10

OBJEXT			= .obj

MCS			= csc -nologo

MCSFLAGS = -d:MAKEFILE_BUILD -target:library -keyfile:$(KEYFILE) -warnaserror+

#
# Supress EnvDTE redirection warning.
#
MCSFLAGS = $(MCSFLAGS) -nowarn:1701

!if "$(DEBUG)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -debug -define:DEBUG
!endif

!if "$(OPTIMIZE)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -optimize+
!endif

!if "$(VS)" == "VS2008"
MCSFLAGS = $(MCSFLAGS) -define:VS2008
!endif

MCSFLAGS = $(MCSFLAGS) /reference:"C:\Windows\Microsoft.NET\Framework\v2.0.50727\mscorlib.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSINSTALLDIR)\Common7\IDE\PublicAssemblies\EnvDTE.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSINSTALLDIR)\Common7\IDE\PublicAssemblies\EnvDTE80.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSINSTALLDIR)\Visual Studio Tools for Office\PIA\Office11\Extensibility.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSINSTALLDIR)\Common7\IDE\PublicAssemblies\Microsoft.VisualStudio.CommandBars.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSINSTALLDIR)\Common7\IDE\PublicAssemblies\VSLangProj.dll"

MCSFLAGS = $(MCSFLAGS) /reference:"$(VS_HOME)\Common7\IDE\PublicAssemblies\Microsoft.VisualStudio.VCProject.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VS_HOME)\Common7\IDE\PublicAssemblies\Microsoft.VisualStudio.VCProjectEngine.dll"

MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\Microsoft.VisualStudio.OLE.Interop.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\Microsoft.VisualStudio.Shell.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\Microsoft.VisualStudio.Shell.Interop.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\Microsoft.VisualStudio.Shell.Interop.8.0.dll"

all::
