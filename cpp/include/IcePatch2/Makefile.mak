# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

!include $(top_srcdir)/config/Make.rules.mak

install::
	@if not exist "$(install_includedir)\IcePatch2" \
	    @echo "Creating $(install_includedir)\IcePatch2..." && \
	    $(MKDIR) "$(install_includedir)\IcePatch2"

	@for %i in ( *.h ) do \
	    @echo Installing %i && \
	    copy %i "$(install_includedir)\IcePatch2"
