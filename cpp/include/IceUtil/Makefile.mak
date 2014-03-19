# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..
INCLUDE_DIR	= IceUtil

!include $(top_srcdir)/config/Make.rules.mak

!if "$(WINRT)" != "yes"
install::
	@if not exist "$(install_includedir)\IceUtil" \
	    @echo "Creating $(install_includedir)\IceUtil..." && \
	    mkdir "$(install_includedir)\IceUtil"

	@for %i in ( *.h ) do \
	    @echo Installing %i && \
	    copy %i "$(install_includedir)\IceUtil"

!else

all::
        @echo SDK_HEADERS       =  \> .headers
        @for /f %i in ('dir /b *.h') do \
                @echo ^ $$(SDK_INCLUDE_PATH)\$$(INCLUDE_DIR)\%i \>> .headers


!include .headers

all:: $(SDK_HEADERS)

install:: all

!endif
