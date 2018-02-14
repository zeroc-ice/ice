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
	@if not exist "$(install_includedir)\Slice" \
	    @echo "Creating $(install_includedir)\Slice..." && \
	    $(MKDIR) "$(install_includedir)\Slice"

	@for %i in ( *.h ) do \
	    @echo Installing %i && \
	    copy %i "$(install_includedir)\Slice"
