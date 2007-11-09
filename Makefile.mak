# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

root_dir	= .

!include $(root_dir)/config/Make.rules.mak

SUBDIRS		= slice cpp java

!if "$(CPP_COMPILER)" == "VC60"
SUBDIRS		= $(SUBDIRS) php rb
!endif

!if "$(CPP_COMPILER)" == "VC80"
SUBDIRS		= $(SUBDIRS) cs py vb
!endif

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) $@" || exit 1


