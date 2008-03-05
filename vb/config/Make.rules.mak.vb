# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

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

#
# Common definitions
#
ice_language = vb
slice_translator = slice2cs.exe

!if exist ($(top_srcdir)\..\config\Make.common.rules.mak)
!include $(top_srcdir)\..\config\Make.common.rules.mak
!else
!include $(top_srcdir)\config\Make.common.rules.mak
!endif

!if "$(ice_src_dist)" != ""
csbindir			= $(ice_dir)\cs\bin
!else
csbindir			= $(ice_dir)\bin
!endif

#
# If a slice directory is contained along with this distribution -- use it. 
# Otherwise use paths relative to $(ICE_HOME).
#

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

!if "$(ice_src_dist)" != ""
SLICE2CS		= "$(ice_cpp_dir)\bin\slice2cs.exe"
!else
SLICE2CS		= "$(ice_dir)\bin\slice2cs.exe"
!endif

EVERYTHING		= all clean depend config

.SUFFIXES:
.SUFFIXES:		.cs .vb .ice

.ice.cs:
	$(SLICE2CS) $(SLICE2CSFLAGS) $<

{$(SDIR)\}.ice{$(GDIR)}.cs:
	$(SLICE2CS) --output-dir $(GDIR) $(SLICE2CSFLAGS) $<

!if "$(SLICE_ASSEMBLY)" != ""
$(SLICE_ASSEMBLY): $(GEN_SRCS)
        $(MCS) -target:library -out:$@ -r:$(csbindir)\Ice.dll $(GEN_SRCS)
!endif

!if "$(TARGETS_CONFIG)" != ""
$(TARGETS_CONFIG):
!if "$(ice_src_dist)" != ""
        @echo Generating $(TARGETS_CONFIG) ... && \
        python "$(top_srcdir)/config/makeconfig.py" "$(top_srcdir)\..\cs" $(TARGETS_CONFIG:.exe.config=.exe)
!else
        @echo Generating $(TARGETS_CONFIG) ... && \
        python "$(top_srcdir)/config/makeconfig.py" "$(ice_dir)" $(TARGETS_CONFIG:.exe.config=.exe)
!endif
!endif

all:: $(TARGETS) $(TARGETS_CONFIG) $(SLICE_ASSEMBLY)

config:: $(TARGETS_CONFIG)

clean::
	del /q $(TARGETS) $(TARGETS_CONFIG) $(SLICE_ASSEMBLY) *.pdb

!if "$(SLICE_SRCS)" != ""
depend::
	$(SLICE2CS) --depend $(SLICE2CSFLAGS) $(SLICE_SRCS) | python $(ice_dir)\config\makedepend.py > .depend
!else
depend::
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
