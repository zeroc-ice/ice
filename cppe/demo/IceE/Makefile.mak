# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

!include $(top_srcdir)\config\Make.rules.mak

SUBDIRS		= minimal \
		  latency \
		  throughput \
		  chat

!if "$(EMBEDDED_DEVICE)" == ""
SUBDIRS 	= $(SUBDIRS) \
		  hello \
		  callback \
		  bidir \
		  workqueue
!endif

!if "$(CPP_COMPILER)" != "VC80_EXPRESS"
SUBDIRS		= $(SUBDIRS) MFC
!endif

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
