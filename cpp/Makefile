# Copyright (c) ZeroC, Inc.

top_srcdir      := ..
lang_srcdir     := $(top_srcdir)/$(notdir $(patsubst %/,%,$(dir $(lastword $(abspath $(MAKEFILE_LIST))))))

include $(top_srcdir)/config/Make.rules
include $(lang_srcdir)/config/Make.rules

#
# Load these project first in this order. These projects contains generated headers
# that other projects might depend on so it's important to parse them first for the
# generated headers variable to be defined when we load other projects.
#
load_first = Ice IceBox Glacier2Lib IceGridLib IceStormLib DataStorm

projects :=

# Create projects for all the Slice translators from src/slice2* and src/ice2slice.
slice_translators := $(wildcard $(lang_srcdir)/src/slice2*) $(lang_srcdir)/src/ice2slice
$(foreach t, $(slice_translators), $(eval $(call create-translator-project,$(call project,$t))))

include $(foreach p,$(load_first),src/$p/Makefile.mk)
include $(filter-out $(foreach p,$(load_first),%/src/$p/Makefile.mk),$(shell find $(lang_srcdir)/src -name Makefile.mk))
$(call make-projects,$(projects),make-cpp-src-project,,$(filter all cpp,$(ICE_BIN_DIST)))

ifeq ($(os),Darwin)
$(call make-xcframeworks)
endif

#
# Now, create and load test projects.
#
projects :=
tests := $(call tests-without-project-makefile,cpp)
include $(shell find $(lang_srcdir)/test -name Makefile.mk)
$(foreach t,$(tests),$(eval $(call create-cpp-test-project,$(t))))
$(call make-projects,$(projects),make-cpp-test-project)

#
# Create projects for doxygen examples.
#
projects :=
doxygen_examples := Ice
include $(foreach p,$(doxygen_examples),doxygen/examples/$p/Makefile.mk)
$(call make-projects,$(projects),make-cpp-doxygen-example-project)

install:: | $(DESTDIR)$(install_configdir)
	$(Q)$(call install-data,templates.xml,$(lang_srcdir)/config,$(install_configdir))
