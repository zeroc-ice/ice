# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

#
# Get Make.common.rules.mak to figure out CPP_COMPILER by setting it
# to "auto"
#
CPP_COMPILER=auto

!include $(top_srcdir)\config\Make.rules.mak.php

!if "$(CPP_COMPILER)" == "VC140"
SUBDIRS		= php7
!else
SUBDIRS		= php5
!endif

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
