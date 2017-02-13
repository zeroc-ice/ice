# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	:= ../..
lang_srcdir	:= $(top_srcdir)/python

include $(top_srcdir)/config/Make.rules
include $(lang_srcdir)/config/Make.rules

#
# Load C++ dependencies
#
$(eval $(call load-translator-dependencies,$(top_srcdir)/cpp/src/slice2py))

.NOTPARALLEL:

install:: | $(DESTDIR)$(install_pythondir)
	$(E) "Installing generated code"
	$(Q)$(INSTALL) *.py $(DESTDIR)$(install_pythondir)

$(eval $(call make-python-package,$(slicedir),$(lang_srcdir)/python,Ice,--no-package))
$(eval $(call make-python-package,$(slicedir),$(lang_srcdir)/python,Glacier2,--no-package))
$(eval $(call make-python-package,$(slicedir),$(lang_srcdir)/python,IceBox))
$(eval $(call make-python-package,$(slicedir),$(lang_srcdir)/python,IceGrid))
$(eval $(call make-python-package,$(slicedir),$(lang_srcdir)/python,IcePatch2))
$(eval $(call make-python-package,$(slicedir),$(lang_srcdir)/python,IceStorm))

# Generate this two files individually without the --no-package option to ensure the
# IceMX package is updated.
$(eval $(call make-python-slice,$(slicedir),$(lang_srcdir)/python,Ice,Metrics))
$(eval $(call make-python-slice,$(slicedir),$(lang_srcdir)/python,Glacier2,Metrics))

