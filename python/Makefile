# Copyright (c) ZeroC, Inc.

top_srcdir      := ..
lang_srcdir     := $(top_srcdir)/$(notdir $(patsubst %/,%,$(dir $(lastword $(abspath $(MAKEFILE_LIST))))))

include $(top_srcdir)/config/Make.rules

#
# Load C++ dependencies
#
$(eval $(call load-dependencies,$(addprefix $(top_srcdir)/cpp/src/,Ice IceLocatorDiscovery IceDiscovery)))

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
srcs:: $(projects)

all:: srcs

#
# We use a separate Makefile for building the python packages because their building
# requires .NOTPARALLEL
#
all generate-srcs clean distclean install::
	+$(Q)$(MAKE) -C python $@
