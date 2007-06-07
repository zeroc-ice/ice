# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

!include $(top_srcdir)/config/Make.rules.mak

install::
	@if not exist $(install_includedir)\IceGrid \
	    @echo "Creating $(install_includedir)\IceGrid..." && \
	    mkdir $(install_includedir)\IceGrid

	@for %i in ( *.h ) do \
	    @echo Installing %i && \
	    copy %i $(install_includedir)\IceGrid
