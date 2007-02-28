# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# If you are compiling with MONO you must define this symbol.
#
MONO = yes

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#

prefix			= /opt/IceCS-$(VERSION)

#
# The default behavior of 'make install' attempts to add the Ice for C#
# libraries to the Global Assembly Cache (GAC). If you would prefer not
# to install these libraries to the GAC, or if you do not have sufficient
# privileges to do so, then enable no_gac and the libraries will be
# copied to $(prefix)/bin instead.
#

#no_gac			= 1

#
# Define DEBUG as yes if you want to build with debug information and
# assertions enabled.
#

DEBUG			= yes

#OPTIMIZE		= yes

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------


ifeq ($(MONO), yes)
	DSEP = /
else
	DSEP = \\
endif

SHELL			= /bin/sh
VERSION			= 3.2.0

#
# Checks for ICE_HOME environment variable.  If it isn't present,
# attempt to find an Ice installation in /usr or the default install
# location.
#
ifeq ($(ICE_HOME),)
    ICE_DIR = /usr
    ifneq ($(shell test -f $(ICE_DIR)/bin/slice2cs && echo 0),0)
	NEXTDIR = /opt/Ice-$(VERSION_MAJOR).$(VERSION_MINOR)
	ifneq ($(shell test -f $(NEXTDIR)/bin/slice2cs && echo 0),0)
$(error Unable to locate Ice distribution, please set ICE_HOME!)
	else
	    ICE_DIR = $(NEXTDIR)
	endif
    else
	NEXTDIR = /opt/IceCS-$(VERSION_MAJOR).$(VERSION_MINOR)
	ifeq ($(shell test -f $(NEXTDIR)/bin/slice2cs && echo 0),0)
$(warning Ice distribution found in /usr and $(NEXTDIR)! Installation in "/usr" will be used by default. Use ICE_HOME to specify alternate Ice installation.)
	endif
    endif
else
    ICE_DIR = $(ICE_HOME)
    ifneq ($(shell test -f $(ICE_DIR)/bin/slice2cs && echo 0),0)
$(error Ice distribution not found in $(ICE_DIR), please verify ICE_HOME location!)
    endif
endif

bindir			= $(top_srcdir)/bin
libdir			= $(top_srcdir)/lib

#
# If a slice directory is contained along with this distribution -- use it. 
# Otherwise use paths relative to $(ICE_DIR).
#
ifneq ($(shell test -d $(top_srcdir)/slice && echo 0),0)
    ifeq ($(ICE_DIR),/usr)
	slicedir = $(ICE_DIR)/share/Ice-$(VERSION)/slice
    else
	slicedir = $(ICE_DIR)/slice
    endif
else
    slicedir = $(top_srcdir)/slice
endif

install_bindir		= $(prefix)/bin
install_libdir		= $(prefix)/lib
install_slicedir	= $(prefix)/slice

ifneq ($(ICE_DIR),/usr)
ref = -r:$(bindir)/$(1).dll
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

ifeq ($(MONO),yes)
MCS			= gmcs
else
MCS			= csc -nologo
endif

LIBS			= $(bindir)/icecs.dll $(bindir)/glaciercs.dll

MCSFLAGS = -warnaserror -d:MAKEFILE_BUILD
ifeq ($(DEBUG),yes)
    MCSFLAGS := $(MCSFLAGS) -debug -define:DEBUG
endif

ifeq ($(OPTIMIZE),yes)
    MCSFLAGS := $(MCSFLAGS) -optimize+
endif

ifeq ($(installdata),)
    installdata		= $(INSTALL_DATA) $(1) $(2); \
			  chmod a+r $(2)/$(notdir $(1))
endif

ifeq ($(installprogram),)
    installprogram	= $(INSTALL_PROGRAM) $(1) $(2); \
			  chmod a+rx $(2)/$(notdir $(1))
endif

ifeq ($(installlibrary),)
    installlibrary	= $(INSTALL_LIBRARY) $(1) $(2); \
			  chmod a+rx $(2)/$(notdir $(1))
endif

ifeq ($(mkdir),)
    mkdir		= mkdir $(1) ; \
			  chmod a+rx $(1)
endif

SLICE2CS		= $(ICE_DIR)/bin/slice2cs

GEN_SRCS = $(subst .ice,.cs,$(addprefix $(GDIR)/,$(notdir $(SLICE_SRCS))))
CGEN_SRCS = $(subst .ice,.cs,$(addprefix $(GDIR)/,$(notdir $(SLICE_C_SRCS))))
SGEN_SRCS = $(subst .ice,.cs,$(addprefix $(GDIR)/,$(notdir $(SLICE_S_SRCS))))
GEN_AMD_SRCS = $(subst .ice,.cs,$(addprefix $(GDIR)/,$(notdir $(SLICE_AMD_SRCS))))
SAMD_GEN_SRCS = $(subst .ice,.cs,$(addprefix $(GDIR)/,$(notdir $(SLICE_SAMD_SRCS))))


EVERYTHING		= all depend clean install config

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
	    $(SLICE2CS) --depend $(SLICE2CSFLAGS) $(SLICE_SRCS) | \
	    $(top_srcdir)/config/makedepend.py >> .depend; \
	fi
	if test -n "$(SLICE_C_SRCS)" ; then \
	    $(SLICE2CS) --depend $(SLICE2CSFLAGS) $(SLICE_C_SRCS) | \
	    $(top_srcdir)/config/makedepend.py >> .depend; \
	fi
	if test -n "$(SLICE_S_SRCS)" ; then \
	    $(SLICE2CS) --depend $(SLICE2CSFLAGS) $(SLICE_S_SRCS) | \
	    $(top_srcdir)/config/makedepend.py >> .depend; \
	fi
	if test -n "$(SLICE_AMD_SRCS)" ; then \
	    $(SLICE2CS) --depend $(SLICE2CSFLAGS) $(SLICE_AMD_SRCS) | \
	    $(top_srcdir)/config/makedepend.py >> .depend; \
	fi
	if test -n "$(SLICE_SAMD_SRCS)" ; then \
	    $(SLICE2CS) --depend $(SLICE2CSFLAGS) $(SLICE_SAMD_SRCS) | \
	    $(top_srcdir)/config/makedepend.py >> .depend; \
	fi

clean::
	-rm -f $(TARGETS) $(patsubst %,%.mdb,$(TARGETS)) *.bak *.dll *.pdb *.mdb

config::
	$(top_srcdir)/config/makeconfig.py $(top_srcdir) $(TARGETS)

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
