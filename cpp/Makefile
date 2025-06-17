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
$(call make-projects,$(projects),make-cpp-src-project,,)
srcs:: $(projects)

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
tests:: $(projects)

#
# Create projects for doxygen examples.
#
projects :=
doxygen_examples := Ice
include $(foreach p,$(doxygen_examples),doxygen/examples/$p/Makefile.mk)
$(call make-projects,$(projects),make-cpp-doxygen-example-project)
doxygen_examples:: $(projects)

all:: srcs tests doxygen_examples

install:: | $(DESTDIR)$(install_configdir)
	$(Q)$(call install-data,templates.xml,$(lang_srcdir)/config,$(install_configdir))

install:: install-cmake

# Install CMake files for a platform
# $(call install-cmake-files,$1=platform)
define install-cmake-files
$(eval $(call install-data-files,$(wildcard $(lang_srcdir)/config/*.cmake),$(lang_srcdir)/config,$(install_libdir)$$($(1)_cmakedir)/Ice-$(mmversion),\
	install-cmake-$(1),"Installing CMake files for $(1)"))
install-cmake:: install-cmake-$(1)
endef

ifeq ($(os),Darwin)
  # Only install CMake files for macosx on Darwin.
  # We skip iphoneos and iphonesimulator platforms.
  ifneq ($(filter macosx, $(platforms)),)
    $(eval $(call install-cmake-files,macosx))
  endif
else
  $(foreach p,$(platforms),$(eval $(call install-cmake-files,$p)))
endif
