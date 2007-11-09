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
	@if not exist $(install_slicedir)\Freeze \
	    @echo "Creating $(install_slicedir)\Freeze..." && \
	    mkdir $(install_slicedir)\Freeze

	@for %i in ( *.ice ) do \
	    @echo Installing %i && \
	    copy %i $(install_slicedir)\Freeze
