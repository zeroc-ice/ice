# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

!if "$(ICE_HOME)" == ""
!error ICE_HOME is not defined
!endif

#
# Define DEBUG as yes if you want to build with debug information and
# assertions enabled.
#

#DEBUG			= yes

#
# Define OPTIMIZE as yes if you want to build with optimized.
#

#OPTIMIZE		= yes

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

SHELL			= /bin/sh
VERSION			= 3.2.0

bindir			= $(top_srcdir)\bin
libdir			= $(top_srcdir)\lib

!if exist ($(ICE_HOME)\bin\icecs.dll)
csbindir	 	= $(ICE_HOME)\bin
!else
csbindir 		= $(top_srcdir)\..\icecs\bin
!endif

#
# If a slice directory is contained along with this distribution -- use it. 
# Otherwise use paths relative to $(ICE_HOME).
#
!if exist ($(top_srcdir)\slice)
slicedir 		= $(top_srcdir)\slice
!else
slicedir 		= $(ICE_HOME)\slice
!endif

VBC			= vbc -nologo /r:system.dll

VBCFLAGS = -warnaserror
!if "$(DEBUG)" == "yes"
VBCFLAGS 		= $(VBCFLAGS) -debug+ -define:DEBUG=yes
!endif

!if "$(OPTIMIZE)" == "yes"
VBCFLAGS 		= $(VBCFLAGS) -optimize+
!endif

SLICE2VB		= "$(ICE_HOME)\bin\slice2vb"

EVERYTHING		= all clean depend config

.SUFFIXES:
.SUFFIXES:		.vb .ice

.ice.vb:
	$(SLICE2VB) $(SLICE2VBFLAGS) $<

{$(SDIR)\}.ice{$(GDIR)}.vb:
	$(SLICE2VB) --output-dir $(GDIR) $(SLICE2VBFLAGS) $<

all:: $(TARGETS)

clean::
	del /q $(TARGETS) *.pdb


!if "$(SLICE_SRCS)" != ""
depend::
	$(SLICE2VB) --depend $(SLICE2VBFLAGS) $(SLICE_SRCS) | python $(top_srcdir)\config\makedepend.py > .depend
!else
depend::
!endif


!if "$(TARGETS)" != ""
config::
	python $(top_srcdir)\config\makeconfig.py $(top_srcdir) $(TARGETS)
!else
config::
!endif

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
