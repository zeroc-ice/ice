# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir := .

include $(top_srcdir)/config/Make.rules

define make-global-rule
$1::
	+@for subdir in $2; \
	do \
		echo "making all in $$$$subdir"; \
		( cd $$$$subdir && $(MAKE) $1 ) || exit 1; \
	done
endef

$(eval $(call make-global-rule,srcs,$(languages)))
$(eval $(call make-global-rule,tests,$(languages)))
$(eval $(call make-global-rule,all,$(languages)))
$(eval $(call make-global-rule,clean,$(languages)))
$(eval $(call make-global-rule,distclean,$(languages)))
$(eval $(call make-global-rule,install,$(languages)))

#
# Install documentation and slice files (don't install IceDiscovery and IceLocatorDiscovery slice files)
#
install:: install-doc install-slice

$(eval $(call install-data-files,$(filter-out %Discovery.ice,$(wildcard $(slicedir)/*/*.ice)),$(slicedir),\
         $(install_slicedir), install-slice,"Installing slice files"))

$(eval $(call install-data-files,$(wildcard $(top_srcdir)/*LICENSE),$(top_srcdir),$(install_docdir),\
         install-doc,"Installing documentation files"))

#
# Create a symlink for the slice directory. We skip this step on macOS
#
ifneq ($(usr_dir_install),)
ifeq ($(filter Darwin,$(os)),)

install-slice:: $(DESTDIR)$(prefix)/share/slice

$(DESTDIR)$(prefix)/share/slice:
	$(foreach dir,$(notdir $(wildcard slice/*)), \
		$(shell $(MKDIR) -p $(DESTDIR)$(prefix)/share/slice && cd $(DESTDIR)$(prefix)/share/slice && ln -sf ../ice/slice/$(dir) .))
endif
endif

#
# Remove IceSDK directory on macOS
#
ifneq ($(filter Darwin,$(os)),)
distclean::
	$(Q)$(RM) -r $(top_srcdir)/IceSDK
endif
