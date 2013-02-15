# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

DSEP = /

ifeq ($(ice_dir),/usr)
    ref = -pkg:$(1)
else
    ifeq ($(shell test -d $(ice_dir)/lib/pkgconfig && echo 0),0)
        export PKG_CONFIG_PATH := $(ice_dir)/lib/pkgconfig:$(PKG_CONFIG_PATH)
        ref = -pkg:$(1)
    else
        ref = -r:$(ice_dir)/bin/$(1).dll
    endif
endif

MCS			= gmcs

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

%.cs: %.ice
	$(SLICE2CS) $(SLICE2CSFLAGS) $<

$(GDIR)/%.cs: $(SDIR)/%.ice
	$(SLICE2CS) --output-dir $(GDIR) $(SLICE2CSFLAGS) $<

all:: $(TARGETS)

clean::
	-rm -f $(TARGETS) $(patsubst %,%.mdb,$(TARGETS)) *.bak *.dll *.pdb *.mdb

ifneq ($(SLICE_SRCS),)
clean::
	-rm -f $(GEN_SRCS)
endif
