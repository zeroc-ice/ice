# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
# The "root directory" for runpath embedded in executables. Can be unset
# to avoid adding a runpath to Ice executables.
#
embedded_runpath_prefix  ?= /opt/Ice-$(VERSION_MAJOR).$(VERSION_MINOR)

#
# Define OPTIMIZE as yes if you want to build with optimization.
# Otherwise the Ice extension is built with debug information.
#

#OPTIMIZE		= yes

#
# Define LP64 as yes if you want to build in 64 bit mode on a platform
# that supports both 32 and 64 bit.
#
#LP64			:= yes

#
# The values below can be overridden by defining them as environment
# variables.
#

#
# Determines whether the extension uses PHP namespaces (requires
# PHP 5.3 or later).
#
#USE_NAMESPACES          ?= yes

#
# If you've built PHP yourself then set PHP_HOME to contain the
# installation directory; the rest of the PHP-related settings
# should be correct.
#
# If you've installed a distribution, or PHP was included with
# your system, then you likely need to review the values of
# PHP_INCLUDE_DIR and PHP_LIB_DIR.
#

PHP_HOME		?= /opt/php

#
# Verifies the PHP_HOME is valid and attempts to adjust for platform variances
# in install directories.  SuSE installs into 'php5' while Redhat installs to
# php.
#
ifeq ($(shell test -d $(PHP_HOME) && echo 0),0)
    ifeq ($(shell test -d $(PHP_HOME)/include/php5 && echo 0),0)
        PHP_INCLUDE_DIR	= $(PHP_HOME)/include/php5
        PHP_LIB_DIR	= $(PHP_HOME)/lib$(lp64suffix)/php5
    else
        PHP_INCLUDE_DIR	= $(PHP_HOME)/include/php
        PHP_LIB_DIR	= $(PHP_HOME)/lib$(lp64suffix)/php
    endif
else
    ifeq ($(shell test -d /usr/include/php5 && echo 0),0)
        PHP_INCLUDE_DIR	= /usr/include/php5
        PHP_LIB_DIR	= /usr/lib$(lp64suffix)/php5
    else
        PHP_INCLUDE_DIR	= /usr/include/php
        PHP_LIB_DIR	= /usr/lib$(lp64suffix)/php
    endif
endif

PHP_FLAGS ?= -I$(PHP_INCLUDE_DIR) -I$(PHP_INCLUDE_DIR)/main -I$(PHP_INCLUDE_DIR)/Zend -I$(PHP_INCLUDE_DIR)/TSRM


# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Common definitions
#
ice_language     = php
ice_require_cpp  = yes
slice_translator = slice2php

ifeq ($(shell test -f $(top_srcdir)/config/Make.common.rules && echo 0),0)
    include $(top_srcdir)/config/Make.common.rules
else
    include $(top_srcdir)/../config/Make.common.rules
endif

libdir			= $(top_srcdir)/lib
ifneq ($(prefix), /usr)
install_phpdir      = $(prefix)/php
install_libdir      = $(prefix)/php
else
install_phpdir      = $(prefix)/share/php
install_libdir      = $(shell php -r "echo(ini_get('extension_dir'));")
endif

#
# Platform specific definitions
#
ifeq ($(shell test -f $(top_srcdir)/config/Make.rules.$(UNAME) && echo 0),0)
    configdir = $(top_srcdir)/config
else
    configdir = $(top_srcdir)/../cpp/config
endif 
include	$(configdir)/Make.rules.$(UNAME)

ifdef ice_src_dist
    ifeq ($(ice_cpp_dir), $(ice_dir)/cpp)
        ICE_LIB_DIR = -L$(ice_cpp_dir)/lib
    else
        ICE_LIB_DIR = -L$(ice_cpp_dir)/$(libsubdir)
    endif
    ICE_FLAGS 	= -I$(ice_cpp_dir)/include
endif
ifdef ice_bin_dist
    ICE_LIB_DIR = -L$(ice_dir)/lib$(lp64suffix)
    ICE_FLAGS	= -I$(ice_dir)/include
endif
ICE_LIBS = $(ICE_LIB_DIR) -lIce -lSlice -lIceUtil

ifneq ($(embedded_runpath_prefix),)
    runpath_libdir      := $(embedded_runpath_prefix)/lib$(lp64suffix)
endif

CPPFLAGS		=
ICECPPFLAGS		= -I$(slicedir)
SLICE2PHPFLAGS		= $(ICECPPFLAGS)
LDFLAGS			= $(LDPLATFORMFLAGS) $(CXXFLAGS) -L$(libdir)

ifeq ("$(USE_NAMESPACES)","yes")
    CPPFLAGS            := $(CPPFLAGS) -DICEPHP_USE_NAMESPACES
    SLICE2PHPFLAGS      := $(SLICE2PHPFLAGS) -n
endif

ifdef ice_src_dist
    ifeq ($(ice_cpp_dir), $(ice_dir)/cpp)
        SLICE2PHP 	= $(ice_cpp_dir)/bin/slice2php
	SLICEPARSERLIB	= $(ice_cpp_dir)/lib/$(call mklibfilename,Slice,$(VERSION))
    else
        SLICE2PHP 	= $(ice_cpp_dir)/$(binsubdir)/slice2php
	SLICEPARSERLIB	= $(ice_cpp_dir)/$(libsubdir)/$(call mklibfilename,Slice,$(VERSION))
    endif
else
    SLICE2PHP 		= $(ice_dir)/$(binsubdir)/slice2php
    SLICEPARSERLIB	= $(ice_dir)/$(libsubdir)/$(call mklibfilename,Slice,$(VERSION))
endif

ifeq ($(installphplib),)
    installphplib	= $(INSTALL) $(1) $(2); \
			  chmod a+rx $(2)/$(notdir $(1))
endif

#
# We don't need the "lib" prefix.
#
mkphplibname		= $(subst lib,,$(call mklibname,$(1)))

EVERYTHING		= all depend clean install

.SUFFIXES:
.SUFFIXES:		.cpp .o .py .php

all:: $(SRCS)

%.php: $(SDIR)/%.ice
	rm -f $(*F).php
	$(SLICE2PHP) $(SLICE2PHPFLAGS) $<

.cpp.o:
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

clean::
	-rm -f $(TARGETS)
	-rm -f core *.o *.bak

all:: $(SRCS) $(TARGETS)

ifneq ($(SLICE_SRCS),)
depend:: $(SLICE_SRCS)
	rm -f .depend .depend.mak
	$(SLICE2PHP) --depend $(SLICE2PHPFLAGS) $(SLICE_SRCS) | $(ice_dir)/config/makedepend.py
else
depend::
endif

ifneq ($(TEMPLATE_REPOSITORY),)
clean::
	rm -fr $(TEMPLATE_REPOSITORY)
endif

install::
