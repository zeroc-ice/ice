# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	:= ..
lang_srcdir 	:= $(top_srcdir)/objective-c

include $(top_srcdir)/config/Make.rules
include $(lang_srcdir)/config/Make.rules

#
# Load C++ dependencies
#
$(eval $(call load-translator-dependencies,$(top_srcdir)/cpp/src/slice2objc))
$(eval $(call load-dependencies,$(addprefix $(top_srcdir)/cpp/src/,IceUtil Slice Ice IceSSL)))

#
# Load source projects
#
projects :=
include $(shell find $(lang_srcdir)/src -name Makefile.mk)
$(call make-projects,$(projects),make-objc-src-project)

#
# Now, create and load test projects.
#
projects :=
tests := $(tests-without-project-makefile)
include $(shell find $(lang_srcdir)/test -name Makefile.mk)
$(foreach t,$(tests),$(eval $(call create-objc-test-project,$(t))))
$(call make-projects,$(projects),make-objc-test-project)

