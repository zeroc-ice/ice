# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..
INCLUDE_DIR	= IceGrid

!include $(top_srcdir)/config/Make.rules.mak

!if "$(WINRT)" != "yes"

install::
	@if not exist "$(install_includedir)\IceGrid" \
	    @echo "Creating $(install_includedir)\IceGrid..." && \
	    mkdir "$(install_includedir)\IceGrid"

	@for %i in ( *.h ) do \
	    @echo Installing %i && \
	    copy %i "$(install_includedir)\IceGrid"

!else

SDK_HEADERS	= $(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Admin.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Descriptor.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Exception.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\FileParser.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\IceGrid.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Locator.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Observer.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Query.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Registry.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Session.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\UserAccountMapper.h

all::	$(SDK_HEADERS)
!endif
