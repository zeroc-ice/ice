# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..
INCLUDE_DIR	= Glacier2

!include $(top_srcdir)/config/Make.rules.mak

!if "$(WINRT)" != "yes"

install::
	@if not exist "$(install_includedir)\Glacier2" \
	    @echo "Creating $(install_includedir)\Glacier2..." && \
	    $(MKDIR) "$(install_includedir)\Glacier2"

	@for %i in ( *.h ) do \
	    @echo Installing %i && \
	    copy %i "$(install_includedir)\Glacier2"

!else

all::
        @echo SDK_HEADERS       =  \> .headers
        @for /f %i in ('dir /b *.h') do \
            @echo ^ $$(SDK_INCLUDE_PATH)\$$(INCLUDE_DIR)\%i \>> .headers


!include .headers

all:: $(SDK_HEADERS)

install:: all

!endif
