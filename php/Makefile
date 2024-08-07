#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

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

ifeq ($(filter all php,$(ICE_BIN_DIST)),)

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

install:: | $(DESTDIR)$(install_phpdir)
	$(E) "Installing generated code"
	$(Q)$(INSTALL) -m 644 lib/Ice.php $(DESTDIR)$(install_phpdir)

$(eval $(call make-php-package,$(slicedir),lib,Ice))

endif

#
# Translate the Slice files from test directories
#
tests := $(call tests-without-project-makefile,php)
include $(shell find test -name Makefile.mk)
$(foreach t,$(tests),$(eval $(call make-php-test-project,$(t))))
