# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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
	    mkdir "$(install_includedir)\IceStorm"

	@for %i in ( *.h ) do \
	    @echo Installing %i && \
	    copy %i "$(install_includedir)\IceStorm"

!else

SDK_HEADERS	= $(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\IceStorm.h

all::	$(SDK_HEADERS)
!endif
