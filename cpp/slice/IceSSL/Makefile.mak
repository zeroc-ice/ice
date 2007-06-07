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
	@if not exist $(install_slicedir)\IceSSL \
	    @echo "Creating $(install_slicedir)\IceSSL..." && \
	    mkdir $(install_slicedir)\IceSSL

	@for %i in ( *.ice ) do \
	    @echo Installing %i && \
	    copy %i $(install_slicedir)\IceSSL
