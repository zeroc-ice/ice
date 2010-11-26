# **********************************************************************
#
# Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
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
ice_language = cs

!if exist ($(top_srcdir)\..\config\Make.common.rules.mak)
!include $(top_srcdir)\..\config\Make.common.rules.mak
!else
!include $(top_srcdir)\config\Make.common.rules.mak
!endif

EVERYTHING      = all install clean

#
# Visual Studio version
#

!if "$(VSSDK100INSTALL)" != ""
VSTARGET                = VS2010
VSSDK_HOME              = $(VSSDK100INSTALL)
!elseif "$(VSSDK90INSTALL)" != ""
VSTARGET                = VS2008
VSSDK_HOME		= $(VSSDK90INSTALL)
!else
!error Neither VSSDK100INSTALL not VSSDK90INSTALL is set
!endif

VS_HOME 		= $(VSINSTALLDIR)
PKG_PREFIX 		= $(VSTARGET)

bindir 			= ..\bin

install_bindir 		= $(prefix)\bin
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

!if "$(VSTARGET)" == "VS2008"
MCSFLAGS = $(MCSFLAGS) -define:VS2008
!endif

!if "$(VSTARGET)" == "VS2010"
MCSFLAGS = $(MCSFLAGS) -define:VS2010
!endif


MCSFLAGS = $(MCSFLAGS) /reference:"$(VSINSTALLDIR)\Common7\IDE\PublicAssemblies\EnvDTE.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSINSTALLDIR)\Common7\IDE\PublicAssemblies\EnvDTE80.dll"

MCSFLAGS = $(MCSFLAGS) /reference:"$(VSINSTALLDIR)\Common7\IDE\PublicAssemblies\Microsoft.VisualStudio.CommandBars.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSINSTALLDIR)\Common7\IDE\PublicAssemblies\VSLangProj.dll"

MCSFLAGS = $(MCSFLAGS) /reference:"$(VS_HOME)\Common7\IDE\PublicAssemblies\Microsoft.VisualStudio.VCProject.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VS_HOME)\Common7\IDE\PublicAssemblies\Microsoft.VisualStudio.VCProjectEngine.dll"


!if "$(VSTARGET)" == "VS2008"

MCSFLAGS = $(MCSFLAGS) /reference:"$(VSINSTALLDIR)\Visual Studio Tools for Office\PIA\Office11\Extensibility.dll"

MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\Microsoft.VisualStudio.OLE.Interop.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\Microsoft.VisualStudio.Shell.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\Microsoft.VisualStudio.Shell.Interop.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\Microsoft.VisualStudio.Shell.Interop.8.0.dll"

!endif

!if "$(VSTARGET)" == "VS2010"

MCSFLAGS = $(MCSFLAGS) /reference:"$(VSINSTALLDIR)\Visual Studio Tools for Office\PIA\Common\Extensibility.dll"

MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\v2.0\Microsoft.VisualStudio.OLE.Interop.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\v2.0\Microsoft.VisualStudio.Shell.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\v2.0\Microsoft.VisualStudio.Shell.Interop.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(VSSDK_HOME)\VisualStudioIntegration\Common\Assemblies\v2.0\Microsoft.VisualStudio.Shell.Interop.8.0.dll"

!endif

all::
