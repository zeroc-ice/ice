# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

!if "$(ICE_HOME)" == ""
!error Ice distribution not found, please set ICE_HOME!
!endif

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#

prefix			= C:\IceCS-$(VERSION)

#
# The default behavior of 'nmake /f Makefile.mak install' attempts to add
# the Ice for C# libraries to the Global Assembly Cache (GAC). If you would
# prefer not to install these libraries to the GAC, or if you do not have
# sufficient privileges to do so, then enable no_gac and the libraries will
# be copied to $(prefix)/bin instead.
#

#no_gac			= 1

#
# Comment out the following line if building with regular C# compiler.
#
SILVERLIGHT		= yes

#
# Define DEBUG as yes if you want to build with debug information and
# assertions enabled.
#

DEBUG			= yes

#
# Define OPTIMIZE as yes if you want to build with optimized.
#

#OPTIMIZE		= yes

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

SHELL			= /bin/sh
VERSION			= 3.3.0

bindir			= $(top_srcdir)\bin
libdir			= $(top_srcdir)\lib

slicedir 		= $(top_srcdir)\slice

install_bindir		= $(prefix)\bin
install_libdir		= $(prefix)\lib
install_slicedir	= $(prefix)\slice

!if "$(no_gac)" != ""
NOGAC			= $(no_gac)
!endif

GACUTIL			= gacutil

MCS			= csc -nologo

LIBS			= $(bindir)/icesl.dll

MCSFLAGS = -warnaserror -d:MAKEFILE_BUILD

!if "$(SILVERLIGHT)" == "yes"
MCSFLAGS = $(MCSFLAGS) -d:SILVERLIGHT /nowarn:618 /noconfig /nostdlib+ /reference:"C:\\Program Files\\Microsoft Silverlight\\agclr.dll" /reference:"C:\\Program Files\\Microsoft Silverlight\\mscorlib.dll" /reference:"C:\\Program Files\\Microsoft Silverlight\\System.Core.dll" /reference:"C:\\Program Files\\Microsoft Silverlight\\system.dll" /reference:"C:\\Program Files\\Microsoft Silverlight\\system.silverlight.dll" /reference:"C:\\Program Files\\Microsoft Silverlight\\system.Xml.core.dll"
!endif

!if "$(DEBUG)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -debug -define:DEBUG
!endif

!if "$(OPTIMIZE)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -optimize+
!endif

SLICE2SL		= "$(ICE_HOME)\bin\slice2sl"

EVERYTHING		= all clean install 

.SUFFIXES:
.SUFFIXES:		.cs .ice

.ice.cs:
	$(SLICE2SL) $(SLICE2SLFLAGS) $<

{$(SDIR)\}.ice{$(GDIR)}.cs:
	$(SLICE2SL) --output-dir $(GDIR) $(SLICE2SLFLAGS) $<

all:: $(TARGETS)

clean::
	del /q $(TARGETS) *.pdb

!if "$(GEN_SRCS)" != ""
clean::
	del /q $(GEN_SRCS)
!endif
!if "$(CGEN_SRCS)" != ""
clean::
	del /q $(CGEN_SRCS)
!endif
!if "$(SGEN_SRCS)" != ""
clean::
	del /q $(SGEN_SRCS)
!endif
!if "$(GEN_AMD_SRCS)" != ""
clean::
	del /q $(GEN_AMD_SRCS)
!endif
!if "$(SAMD_GEN_SRCS)" != ""
clean::
	del /q $(SAMD_GEN_SRCS)
!endif

install::
