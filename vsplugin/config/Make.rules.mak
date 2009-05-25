# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

DEBUG = yes
OPTIMIZE = no

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#

prefix					= C:\IceVisualStudioExtension-$(VERSION)


#
# Set the target Visual Studio Version  Supported Versions are:
#	VS90 - Visual Studio 2008
#	VS80 - Visual Studio 2005
#

!if "$(VS)" == ""
VS				= VS90
!endif

#
# Set the location of the Visual Studio 2008
#
#VS90_HOME			= C:\Program Files\Microsoft Visual Studio 9.0
VS90_HOME			= C:\Program Files (x86)\Microsoft Visual Studio 9.0

#
# Set the location of the Visual Studio 2005,  This is only required if you are
# building the extension for Visual Studio 2005
#
#VS80_HOME			= C:\Program Files\Microsoft Visual Studio 8
VS80_HOME 			= C:\Program Files (x86)\Microsoft Visual Studio 8

#
# Set the location of the Visual Studio 2008 SDK
#

#VSSDK_HOME			= C:\Program Files\Microsoft Visual Studio 2008 SDK
VSSDK_HOME			= C:\Program Files (x86)\Microsoft Visual Studio 2008 SDK

#
# Set the key file used to sign assemblies.
#

!if "$(KEYFILE)" == ""
KEYFILE				= $(top_srcdir)\config\IceDevKey.snk
!endif

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

EVERYTHING      = all install clean

!if "$(VS)" == "VS90"
VS_HOME = $(VS90_HOME)
PKG_PREFIX = $(VS)
!else
VS_HOME = $(VS80_HOME)
PKG_PREFIX = $(VS)
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

!if "$(VS)" == "VS90"
MCSFLAGS = $(MCSFLAGS) -define:VS90
!endif

MCSFLAGS = $(MCSFLAGS) /reference:"C:\Windows\Microsoft.NET\Framework\v2.0.50727\mscorlib.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VS90_HOME)\Common7\IDE\PublicAssemblies\EnvDTE.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VS90_HOME)\Common7\IDE\PublicAssemblies\EnvDTE80.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VS90_HOME)\Visual Studio Tools for Office\PIA\Office11\Extensibility.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VS90_HOME)\Common7\IDE\PublicAssemblies\Microsoft.VisualStudio.CommandBars.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VS90_HOME)\Common7\IDE\PublicAssemblies\VSLangProj.dll"

MCSFLAGS = $(MCSFLAGS) /reference:"$(VS_HOME)\Common7\IDE\PublicAssemblies\Microsoft.VisualStudio.VCProject.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VS_HOME)\Common7\IDE\PublicAssemblies\Microsoft.VisualStudio.VCProjectEngine.dll"

MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\Microsoft.VisualStudio.OLE.Interop.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\Microsoft.VisualStudio.Shell.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\Microsoft.VisualStudio.Shell.Interop.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\Microsoft.VisualStudio.Shell.Interop.8.0.dll"

all::
