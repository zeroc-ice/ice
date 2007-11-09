# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

root_dir	= ../..

include $(root_dir)/config/Make.rules

install::
	@if test ! -d $(install_slicedir)/Glacier2 ; \
	then \
	    echo "Creating $(install_slicedir)/Glacier2..." ; \
	    $(call mkdir,$(install_slicedir)/Glacier2) ; \
	fi

	@for i in *.ice ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(install_slicedir)/Glacier2/$$i ; \
	    chmod a+r $(install_slicedir)/Glacier2/$$i ; \
	done
