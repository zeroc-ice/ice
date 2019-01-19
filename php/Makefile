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
$(eval $(call load-dependencies,$(addprefix $(top_srcdir)/cpp/src/,Ice IceSSL IceDiscovery IceLocatorDiscovery)))

#
# Load php rules after loading C++ dependencies
#
include $(lang_srcdir)/config/Make.rules

ifeq ($(filter all php,$(ICE_BIN_DIST)),)

#
# Load Makefile.mk fragments
#
ifeq ($(filter tests,$(MAKECMDGOALS)),)
    ifeq ($(shell [ $$($(PHP_CONFIG) --vernum) -lt 70000 ] && echo 0),0)
        include $(lang_srcdir)/src/php5/Makefile.mk
    else
        include $(lang_srcdir)/src/php7/Makefile.mk
    endif
endif

#
# Make the projects (this expands all the build configuration and defines rules for all
# the components).
#
$(call make-projects,$(projects))

install:: | $(DESTDIR)$(install_phpdir)
	$(E) "Installing generated code"
	$(Q)$(INSTALL) -m 644 lib/Glacier2.php  lib/IceBox.php  lib/IceGrid.php lib/IcePatch2.php lib/IceStorm.php $(DESTDIR)$(install_phpdir)
ifeq ($(USE_NAMESPACES),no)
	$(Q)$(INSTALL) -m 644 lib/Ice_no_ns.php $(DESTDIR)$(install_phpdir)/Ice.php
else
	$(Q)$(INSTALL) -m 644 lib/Ice_ns.php $(DESTDIR)$(install_phpdir)/Ice.php
endif

$(eval $(call make-php-package,$(slicedir),lib,Ice))
$(eval $(call make-php-package,$(slicedir),lib,Glacier2))
$(eval $(call make-php-package,$(slicedir),lib,IceBox))
$(eval $(call make-php-package,$(slicedir),lib,IceGrid))
$(eval $(call make-php-package,$(slicedir),lib,IcePatch2))
$(eval $(call make-php-package,$(slicedir),lib,IceStorm))

endif

#
# Translate the Slice files from test directories
#
tests := $(call tests-without-project-makefile,php)
include $(shell find test -name Makefile.mk)
$(foreach t,$(tests),$(eval $(call make-php-test-project,$(t))))
