# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

lang_srcdir	:= $(abspath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
top_srcdir	:= ..

ifeq ($(PYTHON),)
    PYTHON = python
endif

ifeq ($(filter all python,$(ICE_BIN_DIST)),)

include $(top_srcdir)/config/Make.rules

#
# Load C++ dependencies
#
$(eval $(call load-dependencies,$(addprefix $(top_srcdir)/cpp/src/,Ice IceSSL IceLocatorDiscovery IceDiscovery)))

#
# Load python rules after loading C++ dependencies
#
include $(lang_srcdir)/config/Make.rules

#
# Load Makefile.mk fragments
#
include $(shell find $(lang_srcdir) -name Makefile.mk)

#
# Make the projects (this expands all the build configuration and defines rules for all
# the components).
#
$(call make-projects,$(projects))

#
# We use a separate Makefile for building the python packages because their building
# requires .NOTPARALLEL
#
all generate-srcs clean distclean install::
	+$(Q)$(MAKE) -C python $@

endif
