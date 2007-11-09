# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

!if "$(ICE_HOME)" == ""
ICE_DIR		= $(top_srcdir)\..
USE_SRC_DIR	= 1
!else
ICE_DIR 	= $(ICE_HOME)
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
VERSION			= 3.3.0

bindir			= $(top_srcdir)\bin
libdir			= $(top_srcdir)\lib

!if exist ("$(ICE_DIR)\bin\icecs.dll")
csbindir	 	= $(ICE_DIR)\bin
!else
csbindir 		= $(top_srcdir)\..\cs\bin
!endif

#
# If a slice directory is contained along with this distribution -- use it. 
# Otherwise use paths relative to $(ICE_HOME).
#

!if exist ("$(ICE_DIR)\slice")
slicedir 		= $(ICE_DIR)\slice
!else
slicedir                = $(ICE_DIR)\..\slice
!endif

VBC			= vbc -nologo /r:system.dll

VBCFLAGS = -warnaserror
!if "$(DEBUG)" == "yes"
VBCFLAGS 		= $(VBCFLAGS) -debug+ -define:DEBUG=yes
!endif

MCS			= csc -nologo

MCSFLAGS = -warnaserror -d:MAKEFILE_BUILD
!if "$(DEBUG)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -debug -define:DEBUG
!endif

!if "$(OPTIMIZE)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -optimize+
!endif

!if "$(USE_SRC_DIR)" == "1"
SLICE2CS		= "$(ICE_DIR)\cpp\bin\slice2cs.exe"
!else
SLICE2CS		= "$(ICE_DIR)\bin\slice2cs.exe"
!endif

EVERYTHING		= all clean depend config

.SUFFIXES:
.SUFFIXES:		.cs .vb .ice

.ice.cs:
	$(SLICE2CS) $(SLICE2CSFLAGS) $<

{$(SDIR)\}.ice{$(GDIR)}.cs:
	$(SLICE2CS) --output-dir $(GDIR) $(SLICE2CSFLAGS) $<

all:: $(TARGETS) $(SLICE_ASSEMBLY)

!if "$(SLICE_ASSEMBLY)" != ""
$(SLICE_ASSEMBLY): $(GEN_SRCS)
        $(MCS) -target:library -out:$@ -r:$(csbindir)\icecs.dll $(GEN_SRCS)
!endif

clean::
	del /q $(TARGETS) $(SLICE_ASSEMBLY) *.pdb


!if "$(SLICE_SRCS)" != ""
depend::
	$(SLICE2CS) --depend $(SLICE2CSFLAGS) $(SLICE_SRCS) | python $(top_srcdir)\config\makedepend.py > .depend
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
