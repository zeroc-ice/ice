# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
# Define OPTIMIZE as yes if you want to build with optmization.
#

#OPTIMIZE		= yes

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Common definitions
#
ice_language = cs
slice_translator = slice2cs
include $(top_srcdir)/config/Make.common.rules

ifeq ($(ice_dir),/usr)
    ref = -pkg:$(1)
else
    ifeq ($(shell test -d $(ice_dir)/lib/pkgconfig && echo 0),0)
        export PKG_CONFIG_PATH := $(ice_dir)/lib/pkgconfig:$(PKG_CONFIG_PATH)
        ref = -pkg:$(1)
    else
        ref = -r:$(ice_dir)/Assemblies/$(1).dll
    endif
endif

MCS			= gmcs

ifeq ($(GDIR),)
GDIR        = generated
endif

MCSFLAGS = -warnaserror
ifeq ($(DEBUG),yes)
    MCSFLAGS := $(MCSFLAGS) -debug -define:DEBUG
endif

ifeq ($(OPTIMIZE),yes)
    MCSFLAGS := $(MCSFLAGS) -optimize+
endif

SLICE2CS = $(ice_dir)/$(binsubdir)/slice2cs

GEN_SRCS = $(subst .ice,.cs,$(addprefix $(GDIR)/,$(notdir $(SLICE_SRCS))))

EVERYTHING		= all clean

.SUFFIXES:
.SUFFIXES:		.cs .ice


ifneq ($(GEN_SRCS),)

-include $(GEN_SRCS:$(GDIR)/%.cs=.depend/%.ice.d)

all:: $(GEN_SRCS)

clean::
	-rm -f $(GEN_SRCS)
	-rm -f .depend/*.ice.d
endif

ifneq ($(GEN_C_SRCS),)

-include $(GEN_C_SRCS:$(GDIR)/%.cs=.depend/%.ice.d)

all:: $(GEN_C_SRCS)

clean::
	-rm -f $(GEN_C_SRCS)
	-rm -f .depend/*.ice.d
endif

ifneq ($(GEN_S_SRCS),)

-include $(GEN_S_SRCS:$(GDIR)/%.cs=.depend/%.ice.d)

all:: $(GEN_S_SRCS)

clean::
	-rm -f $(GEN_S_SRCS)
	-rm -f .depend/*.ice.d
endif

ifneq ($(GEN_AMD_SRCS),)

-include $(GEN_AMD_SRCS:$(GDIR)/%.cs=.depend/%.ice.d)

all:: $(GEN_AMD_SRCS)

clean::
	-rm -f $(GEN_AMD_SRCS)
	-rm -f .depend/*.ice.d
endif

ifneq ($(GEN_SAMD_SRCS),)

-include $(GEN_SAMD_SRCS:$(GDIR)/%.cs=.depend/%.ice.d)

all:: $(GEN_SAMD_SRCS)

clean::
	-rm -f $(GEN_SAMD_SRCS)
	-rm -f .depend/*.ice.d
endif

%.cs: %.ice
	$(SLICE2CS) $(SLICE2CSFLAGS) $<
	@mkdir -p .depend
	@$(SLICE2CS) $(SLICE2CSFLAGS) --depend $< > .depend/$(*F).ice.d

$(GDIR)/%.cs: $(SDIR)/%.ice
	$(SLICE2CS) --output-dir $(GDIR) $(SLICE2CSFLAGS) $<
	@mkdir -p .depend
	@$(SLICE2CS) --output-dir $(GDIR) $(SLICE2CSFLAGS) --depend $< > .depend/$(*F).ice.d

all:: $(TARGETS)

clean::
	-rm -f $(TARGETS) $(patsubst %,%.mdb,$(TARGETS)) *.bak *.dll *.pdb *.mdb
