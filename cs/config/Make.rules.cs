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

prefix			?= /opt/Ice-$(VERSION)

#
# The default behavior of 'make install' attempts to add the Ice for .NET
# libraries to the Global Assembly Cache (GAC). If you would prefer not
# to install these libraries to the GAC, or if you do not have sufficient
# privileges to do so, then enable no_gac and the libraries will be
# copied to $(prefix)/bin instead.
#

#no_gac			= 1

#
# Ice invokes unmanaged code to implement the following features:
#
# - Protocol compression
# - Signal processing in the Ice.Application class (Windows only)
# - Monotonic time (Windows only)
#
# Enable MANAGED below if you do not require these features and prefer that
# the Ice run time use only managed code.
#

#MANAGED		= yes

#
# Define DEBUG as yes if you want to build with debug information and
# assertions enabled.
#

DEBUG			= yes

#
# Define OPTIMIZE as yes if you want to build with optmization.
#

#OPTIMIZE		= yes

#
# Set the key file used to sign assemblies.
#

KEYFILE                 ?= $(top_srcdir)/../config/IceDevKey.snk

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Common definitions
#
ice_language = cs
slice_translator = slice2cs

ifeq ($(shell test -f $(top_srcdir)/config/Make.common.rules && echo 0),0)
    include $(top_srcdir)/config/Make.common.rules
else
    include $(top_srcdir)/../config/Make.common.rules
endif

DSEP = /

bindir = $(top_srcdir)/bin

install_bindir		= $(prefix)/bin

ifneq ($(ice_dir),/usr)
    ifdef ice_src_dist
        ref = -r:$(bindir)/$(1).dll
    else
        ref = -r:$(ice_dir)/bin/$(1).dll
    endif
else
    ref = -pkg:$(1)
endif

ifdef no_gac
NOGAC			?= $(no_gac)
endif

INSTALL			= cp -fp
INSTALL_PROGRAM		= ${INSTALL}
INSTALL_LIBRARY		= ${INSTALL}
INSTALL_DATA		= ${INSTALL}

GACUTIL			= gacutil

ifeq ($(NOGAC),)
    ifeq ($(GAC_ROOT),)
        installassembly = $(GACUTIL) -i $(1) -f -package $(2)
    else
        installassembly = $(GACUTIL) -i $(1) -f -package $(2) -root $(GAC_ROOT)
    endif
else
    installassembly 	= $(INSTALL_LIBRARY) $(1) $(install_bindir); \
    			  chmod a+rx $(install_bindir)/$(notdir $(1))
endif


MCS			= gmcs

MCSFLAGS = -warnaserror -d:MAKEFILE_BUILD
ifeq ($(DEBUG),yes)
    MCSFLAGS := $(MCSFLAGS) -debug -define:DEBUG
endif

ifeq ($(OPTIMIZE),yes)
    MCSFLAGS := $(MCSFLAGS) -optimize+
endif

ifdef ice_src_dist
    ifeq ($(ice_cpp_dir), $(ice_dir)/cpp)
        SLICE2CS = $(ice_cpp_dir)/bin/slice2cs
    else
        SLICE2CS = $(ice_cpp_dir)/$(binsubdir)/slice2cs
    endif
else
    SLICE2CS = $(ice_dir)/$(binsubdir)/slice2cs
endif

GEN_SRCS = $(subst .ice,.cs,$(addprefix $(GDIR)/,$(notdir $(SLICE_SRCS))))
CGEN_SRCS = $(subst .ice,.cs,$(addprefix $(GDIR)/,$(notdir $(SLICE_C_SRCS))))
SGEN_SRCS = $(subst .ice,.cs,$(addprefix $(GDIR)/,$(notdir $(SLICE_S_SRCS))))
GEN_AMD_SRCS = $(subst .ice,.cs,$(addprefix $(GDIR)/,$(notdir $(SLICE_AMD_SRCS))))
SAMD_GEN_SRCS = $(subst .ice,.cs,$(addprefix $(GDIR)/,$(notdir $(SLICE_SAMD_SRCS))))


EVERYTHING		= all depend clean install

.SUFFIXES:
.SUFFIXES:		.cs .ice

%.cs: %.ice
	$(SLICE2CS) $(SLICE2CSFLAGS) $<

$(GDIR)/%.cs: $(SDIR)/%.ice
	$(SLICE2CS) --output-dir $(GDIR) $(SLICE2CSFLAGS) $<

all:: $(TARGETS)

depend:: $(SLICE_SRCS) $(SLICE_C_SRCS) $(SLICE_S_SRCS) $(SLICE_AMD_SRCS) $(SLICE_SAMD_SRCS)
	-rm -f .depend
	if test -n "$(SLICE_SRCS)" ; then \
	    $(SLICE2CS) --depend $(SLICE2CSFLAGS) $(SLICE_SRCS) | $(ice_dir)/config/makedepend.py >> .depend; \
	fi
	if test -n "$(SLICE_C_SRCS)" ; then \
	    $(SLICE2CS) --depend $(SLICE2CSFLAGS) $(SLICE_C_SRCS) | $(ice_dir)/config/makedepend.py >> .depend; \
	fi
	if test -n "$(SLICE_S_SRCS)" ; then \
	    $(SLICE2CS) --depend $(SLICE2CSFLAGS) $(SLICE_S_SRCS) | $(ice_dir)/config/makedepend.py >> .depend; \
	fi
	if test -n "$(SLICE_AMD_SRCS)" ; then \
	    $(SLICE2CS) --depend $(SLICE2CSFLAGS) $(SLICE_AMD_SRCS) | $(ice_dir)/config/makedepend.py >> .depend; \
	fi
	if test -n "$(SLICE_SAMD_SRCS)" ; then \
	    $(SLICE2CS) --depend $(SLICE2CSFLAGS) $(SLICE_SAMD_SRCS) | $(ice_dir)/config/makedepend.py >> .depend; \
	fi

clean::
	-rm -f $(TARGETS) $(patsubst %,%.mdb,$(TARGETS)) *.bak *.dll *.pdb *.mdb

ifneq ($(SLICE_SRCS),)
clean::
	-rm -f $(GEN_SRCS)
endif
ifneq ($(SLICE_C_SRCS),)
clean::
	-rm -f $(CGEN_SRCS)
endif
ifneq ($(SLICE_S_SRCS),)
clean::
	-rm -f $(SGEN_SRCS)
endif
ifneq ($(SLICE_AMD_SRCS),)
clean::
	-rm -f $(GEN_AMD_SRCS)
endif
ifneq ($(SLICE_SAMD_SRCS),)
clean::
	-rm -f $(SAMD_GEN_SRCS)
endif

install::
	$(shell [ ! -d $(prefix) ] && (mkdir $(prefix); chmod a+rx $(prefix)))
	$(shell [ ! -d $(install_bindir) ] && (mkdir $(install_bindir); chmod a+rx $(install_bindir)))
