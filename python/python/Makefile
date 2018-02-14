# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir      := ../..
lang_srcdir     := $(top_srcdir)/$(notdir $(patsubst %/,%,$(dir $(abspath $(dir $(abspath $(lastword $(MAKEFILE_LIST))))))))

include $(top_srcdir)/config/Make.rules
include $(lang_srcdir)/config/Make.rules

#
# Load C++ dependencies
#
$(eval $(call load-translator-dependencies,$(top_srcdir)/cpp/src/slice2py))

.NOTPARALLEL:

$(eval $(call make-python-package,$(slicedir),$(lang_srcdir)/python,Ice,--no-package))
$(eval $(call make-python-package,$(slicedir),$(lang_srcdir)/python,Glacier2,--no-package))
$(eval $(call make-python-package,$(slicedir),$(lang_srcdir)/python,IceBox))
$(eval $(call make-python-package,$(slicedir),$(lang_srcdir)/python,IceMX))
$(eval $(call make-python-package,$(slicedir),$(lang_srcdir)/python,IceGrid))
$(eval $(call make-python-package,$(slicedir),$(lang_srcdir)/python,IcePatch2))
$(eval $(call make-python-package,$(slicedir),$(lang_srcdir)/python,IceStorm))

# Generate these two files individually without the --no-package option to ensure the
# IceMX package is updated.
$(eval $(call make-python-slice,$(slicedir),$(lang_srcdir)/python,Ice,Metrics))
$(eval $(call make-python-slice,$(slicedir),$(lang_srcdir)/python,Glacier2,Metrics))

install:: | $(DESTDIR)$(install_pythondir)/Ice
	$(E) "Installing generated code"
	$(Q)$(INSTALL) Ice/__init__.py $(DESTDIR)$(install_pythondir)/Ice
	$(Q)$(INSTALL) Ice/IceFuture.py $(DESTDIR)$(install_pythondir)/Ice

install:: | $(DESTDIR)$(install_pythondir)/Glacier2
	$(Q)$(INSTALL) Glacier2/__init__.py $(DESTDIR)$(install_pythondir)/Glacier2
