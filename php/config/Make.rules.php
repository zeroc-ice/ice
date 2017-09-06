# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
# The "root directory" for runpath embedded in executables. Can be set
# to change the runpath added to Ice executables. The default is
# platform dependent (Linux only, on macOS the embedded runpath is always
# based on @loader_path).
#
#embedded_runpath_prefix ?= /opt/Ice-$(VERSION_MAJOR).$(VERSION_MINOR)

#
# Define embedded_runpath as no if you don't want any RPATH added to
# the executables (Linux only, on macOS we always add a runpath).
#
embedded_runpath 	?= yes

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
# Use specific PHP version
#
PHP_CONFIG		?= php-config

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

libdir		= $(top_srcdir)/lib

ifeq ($(findstring /php/test/,$(abspath $(MAKEFILE_LIST))),)
    ifneq ($(shell type $(PHP_CONFIG) > /dev/null 2>&1 && echo 0),0)
        $(error $(PHP_CONFIG) not found review your PHP installation and ensure $(PHP_CONFIG) is in your PATH)
    endif

    ifndef usr_dir_install
        install_phpdir	= $(prefix)/php
        install_libdir  	= $(prefix)/php
    else
        install_phpdir  	= $(prefix)/share/php
        install_libdir  	= $(shell $(PHP_CONFIG) --extension-dir)
    endif
endif

ifdef ice_src_dist
    RPATH_DIR	= $(LOADER_PATH)/../../cpp/$(libsubdir)
else
    RPATH_DIR	= $(ice_dir)/$(libdir)
endif

install_bindir          = $(prefix)/$(binsubdir)

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
    ICE_LIB_DIR = -L$(ice_cpp_dir)/$(libsubdir)
    ICE_FLAGS 	= -I$(ice_cpp_dir)/include
else
    ICE_LIB_DIR = -L$(ice_dir)/lib$(lp64suffix)
    ICE_FLAGS	= -I$(ice_dir)/include
endif
ICE_LIBS = $(ICE_LIB_DIR) -lIce -lIceSSL -lIceDiscovery -lIceLocatorDiscovery -lSlice -lIceUtil

ifneq ($(embedded_runpath_prefix),)
    runpath_libdir      := $(embedded_runpath_prefix)/lib$(lp64suffix)
endif

ICECPPFLAGS		= -I$(slicedir)
SLICE2PHPFLAGS		:= $(SLICE2PHPFLAGS) $(ICECPPFLAGS)
LDFLAGS			:= $(LDFLAGS) $(LDPLATFORMFLAGS) $(CXXFLAGS) -L$(libdir)

ifeq ("$(USE_NAMESPACES)","yes")
    CPPFLAGS            := $(CPPFLAGS) -DICEPHP_USE_NAMESPACES
    SLICE2PHPFLAGS      := $(SLICE2PHPFLAGS) -n
endif

ifdef ice_src_dist
    SLICE2PHP 	= $(ice_cpp_dir)/$(binsubdir)/slice2php
    SLICEPARSERLIB	= $(ice_cpp_dir)/$(libsubdir)/$(call mklibfilename,Slice,$(VERSION))
    ifeq ($(wildcard $(SLICEPARSERLIB)),)
        SLICEPARSERLIB  = $(ice_cpp_dir)/$(lib64subdir)/$(call mklibfilename,Slice,$(VERSION))
    endif
else
    SLICE2PHP 	= $(ice_dir)/$(binsubdir)/slice2php
    SLICEPARSERLIB	= $(ice_dir)/$(libsubdir)/$(call mksoname,Slice)
    ifeq ($(wildcard $(SLICEPARSERLIB)),)
        SLICEPARSERLIB  = $(ice_dir)/$(lib64subdir)/$(call mksoname,Slice)
    endif
endif

ifeq ($(installphplib),)
    installphplib	= $(INSTALL) $(1) $(2); \
			  chmod a+rx $(2)/$(notdir $(1))
endif

#
# We don't need the "lib" prefix.
#
mkphplibname		= $(subst lib,,$(call mklibname,$(1)))

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.cpp .o .ice .php

$(SLICE_SRCS:.ice=.php): $(SLICE2PHP) $(SLICEPARSERLIB)
all:: $(SLICE_SRCS:.ice=.php)

%.php: %.ice
	$(SLICE2PHP) $(SLICE2PHPFLAGS) $<
	@mkdir -p .depend
	@$(SLICE2PHP) $(SLICE2PHPFLAGS) --depend $< > .depend/$(*F).ice.d

.cpp.o:
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<
	@mkdir -p .depend
	@$(CXX) -DMAKEDEPEND -MM $(CXXFLAGS) $(CPPFLAGS) $< > .depend/$(*F).d

clean::
	-rm -f $(TARGETS)
	-rm -f core *.o *.bak
	-rm -rf .depend

ifneq ($(SLICE_SRCS),)
clean::
	-rm -f $(SLICE_SRCS:.ice=.php)
endif

all:: $(TARGETS)

include $(wildcard .depend/*.d)

install::
