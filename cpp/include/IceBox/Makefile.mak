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
	@if not exist "$(install_includedir)\IceBox" \
	    @echo "Creating $(install_includedir)\IceBox..." && \
	    $(MKDIR) "$(install_includedir)\IceBox"

	@for %i in ( *.h ) do \
	    @echo Installing %i && \
	    copy %i "$(install_includedir)\IceBox"
