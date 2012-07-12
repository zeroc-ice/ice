# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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
	    mkdir "$(install_includedir)\Glacier2"

	@for %i in ( *.h ) do \
	    @echo Installing %i && \
	    copy %i "$(install_includedir)\Glacier2"

!else

SDK_HEADERS	= $(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Application.h \
	$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Glacier2.h \
	$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\PermissionsVerifier.h \
	$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Router.h \
	$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\SSLInfo.h \
	$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Session.h \
	$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\SessionHelper.h

all::	$(SDK_HEADERS)

!endif
