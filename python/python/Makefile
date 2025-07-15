# Copyright (c) ZeroC, Inc.

top_srcdir      := ../..
lang_srcdir     := $(top_srcdir)/$(notdir $(patsubst %/,%,$(dir $(abspath $(dir $(abspath $(lastword $(MAKEFILE_LIST))))))))

include $(top_srcdir)/config/Make.rules
include $(lang_srcdir)/config/Make.rules

#
# Load C++ dependencies
#
$(eval $(call load-translator-dependencies,$(top_srcdir)/cpp/src/slice2py))

.NOTPARALLEL:

$(eval $(call make-python-module,Ice))
$(eval $(call make-python-module,Glacier2))
$(eval $(call make-python-module,IceBox))
$(eval $(call make-python-module,IceGrid))
$(eval $(call make-python-module,IceStorm))

# Make Python packages.

ICE_MX_FILES := $(top_srcdir)/slice/Glacier2/Metrics.ice $(top_srcdir)/slice/Ice/Metrics.ice $(top_srcdir)/slice/IceStorm/Metrics.ice
GLACIER2_FILES := $(filter-out %/Metrics.ice,$(wildcard $(top_srcdir)/slice/Glacier2/*.ice))
ICE_BOX_FILES := $(wildcard $(top_srcdir)/slice/IceBox/*.ice)
ICE_GRID_FILES := $(wildcard $(top_srcdir)/slice/IceGrid/*.ice)
ICE_STORM_FILES := $(filter-out %/Metrics.ice,$(wildcard $(top_srcdir)/slice/IceStorm/*.ice))

$(eval $(call make-python-package,Glacier2,$(GLACIER2_FILES)))
$(eval $(call make-python-package,IceBox,$(ICE_BOX_FILES)))
$(eval $(call make-python-package,IceGrid,$(ICE_GRID_FILES)))
$(eval $(call make-python-package,IceStorm,$(ICE_STORM_FILES)))
$(eval $(call make-python-package,IceMX,$(ICE_MX_FILES)))

$(DESTDIR)$(install_pythondir)/Ice/asyncio:
	$(Q)$(MKDIR) -p $(DESTDIR)$(install_pythondir)/Ice/asyncio

install:: | $(DESTDIR)$(install_pythondir)/Ice/asyncio
	$(E) "Installing Ice python code"
	$(Q)$(INSTALL) -m 644 Ice/*.py $(DESTDIR)$(install_pythondir)/Ice
	$(Q)$(INSTALL) -m 644 Ice/asyncio/*.py $(DESTDIR)$(install_pythondir)/Ice/asyncio
