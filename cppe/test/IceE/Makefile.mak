# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

!include $(top_srcdir)\config\Make.rules.mak

SUBDIRS		= proxy \
                  operations \
		  exceptions \
		  inheritance \
		  facets \
		  location \
		  slicing \
		  custom \
		  retry \
		  thread \
		  uuid

!if "$(EMBEDDED_DEVICE)" == ""
SUBDIRS		= $(SUBDIRS) \
		  faultTolerance \
		  adapterDeactivation \
!endif

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
