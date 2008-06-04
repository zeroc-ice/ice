# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

!include $(top_srcdir)/config/Make.rules.mak

install::
	@if not exist $(install_includedir)\IceE \
	    @echo "Creating $(install_includedir)\IceE..." && \
	    mkdir $(install_includedir)\IceE

	@for %i in ( *.h ) do \
	    @echo Installing %i && \
	    copy %i $(install_includedir)\IceE
