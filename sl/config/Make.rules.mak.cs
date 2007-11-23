# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

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

SILVERLIGHT		= yes

SHELL			= /bin/sh
VERSION			= 0.1.0

bindir			= $(top_srcdir)\bin
libdir			= $(top_srcdir)\lib

slicedir 		= $(top_srcdir)\slice

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

SLICE2SL		= "slice2sl"

EVERYTHING		= all clean

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
