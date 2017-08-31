# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir      := ..
lang_srcdir     := $(top_srcdir)/$(notdir $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST))))))

include $(top_srcdir)/config/Make.rules
include $(lang_srcdir)/config/Make.rules

#
# Load C++ dependencies
#
$(eval $(call load-translator-dependencies,$(top_srcdir)/cpp/src/slice2objc))
$(eval $(call load-dependencies,$(addprefix $(top_srcdir)/cpp/src/,Ice IceSSL IceIAP)))

#
# Load source projects
#
projects :=
include $(shell find $(lang_srcdir)/src -name Makefile.mk)
$(call make-projects,$(projects),make-objc-src-project,,$(filter all cpp,$(ICE_BIN_DIST)))

#
# Now, create and load test projects.
#
projects :=
tests := $(call tests-without-project-makefile,m)
include $(shell find $(lang_srcdir)/test -name Makefile.mk)
$(foreach t,$(tests),$(eval $(call create-objc-test-project,$(t))))
$(call make-projects,$(projects),make-objc-test-project)
