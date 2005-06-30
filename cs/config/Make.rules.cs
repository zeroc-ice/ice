# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Set this to the root of the Ice for C++ source tree if you want to
# build using the Slice definitions from the C++ source tree.
# (You must set slice_home to a pathname that is relative to $(top_srcdir),
# which is the current directory.) If you do not have the Ice for C++
# source tree around, the setting of slice_home does not matter.
#

slice_home=$(top_srcdir)/../ice

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#

prefix			= /opt/icecs-$(VERSION)

#
# Define DEBUG as yes if you want to build with debug information and
# assertions enabled.
#

DEBUG		= yes

#
# If libbzip2 is not installed in a standard location where the compiler 
# can find it, set BZIP2_HOME to the bzip2 installation directory.
#

#BZIP2_HOME		?= /opt/bzip2

#
# If Berkeley DB is not installed in a standard location where the
# compiler can find it, set DB_HOME to the Berkeley DB installation
# directory.
#

#DB_HOME		?= /opt/db

#
# If OpenSSL is not installed in a standard location where the
# compiler can find it, set OPENSSL_HOME to the OpenSSL installation
# directory.
#

#OPENSSL_HOME		?= /opt/openssl

#
# Define if your OpenSSL requires Kerberos, and if Kerberos is not
# installed in a standard location.
#

KERBEROS_HOME		?= /usr/kerberos

#
# If expat is not installed in a standard location where the
# compiler can find it, set EXPAT_HOME to the expat
# installation directory.
#

#EXPAT_HOME		?= /opt/expat

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

SHELL			= /bin/sh
VERSION			= 2.2.0
bindir			= $(top_srcdir)/bin
libdir			= $(top_srcdir)/lib
slicedir := $(shell test -d $(top_srcdir)/slice && echo $(top_srcdir))
ifdef slicedir
slicedir := $(slicedir)/slice
endif
ifndef slicedir
slicedir := $(slice_home)/slice
endif
install_bindir		= $(prefix)/bin
install_libdir		= $(prefix)/lib
install_slicedir	= $(prefix)/slice

INSTALL			= cp -fp
INSTALL_PROGRAM		= ${INSTALL}
INSTALL_LIBRARY		= ${INSTALL}
INSTALL_DATA		= ${INSTALL}

GACUTIL			= gacutil

MCS			= mcs

LIBS			= $(bindir)/icecs.dll $(bindir)/glaciercs.dll

MCSFLAGS = -warnaserror
ifeq ($(DEBUG),yes)
    MCSFLAGS := $(MCSFLAGS) -debug -define:DEBUG
endif

ifeq ($(installdata),)
    installdata		= $(INSTALL_DATA) $(1) $(2); \
			  chmod a+r $(2)/$(notdir $(1))
endif

ifeq ($(installprogram),)
    installprogram	= $(INSTALL_PROGRAM) $(1) $(2); \
			  chmod a+rx $(2)/$(notdir $(1))
endif

ifeq ($(mkdir),)
    mkdir		= mkdir $(1) ; \
			  chmod a+rx $(1)
endif

SLICE2CS		= slice2cs
SLICE2CSFLAGS		=

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
	echo "slicedir: $(slicedir)"
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
	-rm -f $(TARGETS) *.bak *.dll *.pdb

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
