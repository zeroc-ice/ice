# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..
INCLUDE_DIR	= IceStorm

!include $(top_srcdir)/config/Make.rules.mak

!if "$(WINRT)" != "yes"

install::
	@if not exist "$(install_includedir)\IceStorm" \
	    @echo "Creating $(install_includedir)\IceStorm..." && \
	    $(MKDIR) "$(install_includedir)\IceStorm"

	@for %i in ( *.h ) do \
	    @echo Installing %i && \
	    copy %i "$(install_includedir)\IceStorm"

!else

!include .headers

all:: $(SDK_HEADERS)

install:: all

!endif
