# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

root_dir	= ..\..

!include $(root_dir)/config/Make.rules.mak

install::
	@if not exist $(install_slicedir)\Ice \
	    @echo "Creating $(install_slicedir)\Ice..." && \
	    mkdir $(install_slicedir)\Ice

	@for %i in ( *.ice ) do \
	    @echo Installing %i && \
	    copy %i $(install_slicedir)\Ice
