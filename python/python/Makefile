# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
	$(Q)$(INSTALL) -m 644 Ice/__init__.py $(DESTDIR)$(install_pythondir)/Ice
# Ice/Future requires python >= 3.5
ifeq ($(shell $(PYTHON) -c "print(1 if $(python-version) >= 3.5 else 0)"),1)
	$(Q)$(INSTALL) -d -m 755 $(DESTDIR)$(install_pythondir)/Ice/Py3
	$(Q)$(INSTALL) -m 644 Ice/Py3/IceFuture.py $(DESTDIR)$(install_pythondir)/Ice/Py3/
endif

install:: | $(DESTDIR)$(install_pythondir)/Glacier2
	$(Q)$(INSTALL) -m 644 Glacier2/__init__.py $(DESTDIR)$(install_pythondir)/Glacier2
