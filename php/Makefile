# Copyright (c) ZeroC, Inc.

top_srcdir      := ..
lang_srcdir     := $(top_srcdir)/$(notdir $(patsubst %/,%,$(dir $(lastword $(abspath $(MAKEFILE_LIST))))))

include $(top_srcdir)/config/Make.rules

#
# Load C++ dependencies
#
$(eval $(call load-translator-dependencies,$(top_srcdir)/cpp/src/slice2php))
$(eval $(call load-dependencies,$(addprefix $(top_srcdir)/cpp/src/,Ice IceDiscovery IceLocatorDiscovery)))

#
# Load php rules after loading C++ dependencies
#
include $(lang_srcdir)/config/Make.rules

#
# Load Makefile.mk fragments
#
ifeq ($(filter tests,$(MAKECMDGOALS)),)
    include $(lang_srcdir)/src/Makefile.mk
endif

#
# Make the projects (this expands all the build configuration and defines rules for all
# the components).
#
$(call make-projects,$(projects))
srcs:: $(projects)

install:: | $(DESTDIR)$(install_phpdir)
	$(E) "Installing generated code"
	$(Q)$(INSTALL) -m 644 lib/Ice.php $(DESTDIR)$(install_phpdir)

$(eval $(call make-php-package,$(slicedir),lib,Ice))

#
# Translate the Slice files from test directories
#
tests := $(call tests-without-project-makefile,php)
include $(shell find test -name Makefile.mk)
$(foreach t,$(tests),$(eval $(call make-php-test-project,$(t))))

all:: srcs tests
