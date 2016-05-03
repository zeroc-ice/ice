# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	:= ..
lang_srcdir	:= $(top_srcdir)/php

include $(top_srcdir)/config/Make.rules

#
# Load C++ dependencies
#
$(eval $(call load-translator-dependencies,$(top_srcdir)/cpp/src/slice2php))
$(eval $(call load-dependencies,$(addprefix $(top_srcdir)/cpp/src/,IceUtil Slice Ice IceSSL IceDiscovery IceLocatorDiscovery)))

#
# Load php rules after loading C++ dependencies
#
include $(lang_srcdir)/config/Make.rules

#
# Load Makefile.mk fragments
#
ifeq ($(shell [ $$(php-config --vernum) -lt 70000 ] && echo true),true)
include $(lang_srcdir)/src/php5/Makefile.mk
else
include $(lang_srcdir)/src/php7/Makefile.mk
endif

#
# Make the projects (this expands all the build configuration and defines rules for all
# the components).
#
$(foreach p,$(projects),$(eval $(call make-project,$(p))))

install:: | $(DESTDIR)$(install_phpdir)
	$(E) "Installing generated code"
	$(Q)$(INSTALL) $(lang_srcdir)/lib/*.php $(DESTDIR)$(install_phpdir)

$(eval $(call make-php-package,$(slicedir),$(lang_srcdir)/lib,Ice))
$(eval $(call make-php-package,$(slicedir),$(lang_srcdir)/lib,Glacier2))
$(eval $(call make-php-package,$(slicedir),$(lang_srcdir)/lib,IceBox))
$(eval $(call make-php-package,$(slicedir),$(lang_srcdir)/lib,IceGrid))
$(eval $(call make-php-package,$(slicedir),$(lang_srcdir)/lib,IcePatch2))
$(eval $(call make-php-package,$(slicedir),$(lang_srcdir)/lib,IceStorm))
