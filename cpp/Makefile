# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

lang_srcdir	:= $(abspath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
top_srcdir	:= $(abspath $(lang_srcdir)/../)

include $(top_srcdir)/config/Make.rules
include $(lang_srcdir)/config/Make.rules

#
# Create projects for all the Slice translators from src/slice2* and load source projects.
#
projects :=
$(foreach t,$(wildcard $(lang_srcdir)/src/slice2*),$(eval $(call create-translator-project,$(call project,$t))))
include $(shell find $(lang_srcdir)/src -name Makefile.mk)
$(call make-projects,$(projects),make-cpp-src-project)

#
# Now, create and load test projects.
#
projects :=
tests := $(call tests-without-project-makefile,cpp)
include $(shell find $(lang_srcdir)/test -name Makefile.mk)
$(foreach t,$(tests),$(eval $(call create-cpp-test-project,$(t))))
$(call make-projects,$(projects),make-cpp-test-project)

install:: | $(DESTDIR)$(install_configdir)
	$(Q)$(INSTALL) $(lang_srcdir)/config/templates.xml $(DESTDIR)$(install_configdir)
