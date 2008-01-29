# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#

prefix			= C:\Ice-$(VERSION)

#
# The default behavior of 'nmake /f Makefile.mak install' attempts to add
# the Ice for .NET libraries to the Global Assembly Cache (GAC). If you would
# prefer not to install these libraries to the GAC, or if you do not have
# sufficient privileges to do so, then enable no_gac and the libraries will
# be copied to $(prefix)/bin instead.
#

#no_gac			= 1

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

# Setup some variables for Make.rules.common
ice_language = cs
slice_translator = slice2cs.exe

!if exist ($(top_srcdir)\..\config\Make.common.rules.mak)
!include $(top_srcdir)\..\config\Make.common.rules.mak
!else
!include $(top_srcdir)\config\Make.common.rules.mak
!endif

SHELL			= /bin/sh
VERSION			= 3.3.0

!if "$(ice_src_dist)" != ""
bindir			= $(ice_dir)\cs\bin
!else
bindir			= $(ice_dir)\bin
!endif

slicedir                = $(ice_dir)\slice

install_bindir		= $(prefix)\bin
install_libdir		= $(prefix)\lib
install_slicedir	= $(prefix)\slice

!if "$(no_gac)" != ""
NOGAC			= $(no_gac)
!endif

GACUTIL			= gacutil

MCS			= csc -nologo

LIBS			= $(bindir)/icecs.dll $(bindir)/glaciercs.dll

MCSFLAGS = -warnaserror -d:MAKEFILE_BUILD
!if "$(DEBUG)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -debug -define:DEBUG
!endif

!if "$(OPTIMIZE)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -optimize+
!endif

!if "$(ice_src_dist)" != ""
SLICE2CS		= "$(ice_cpp_dir)\bin\slice2cs.exe"
!else
SLICE2CS		= "$(ice_dir)\bin\slice2cs.exe"
!endif

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.cs .ice

.ice.cs:
	$(SLICE2CS) $(SLICE2CSFLAGS) $<

{$(SDIR)\}.ice{$(GDIR)}.cs:
	$(SLICE2CS) --output-dir $(GDIR) $(SLICE2CSFLAGS) $<

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
