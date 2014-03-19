# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
		  info \
		  inheritance \
		  facets \
		  objects \
		  location \
		  adapterDeactivation \
		  hash \
		  ami \
		  invoke \
		  dispatcher \
		  hold \
		  binding \
		  retry \
		  timeout \
		  udp \
		  admin \
		  plugin \
		  stream \
		  metrics \
		  optional \
		  enums \
		  echo

!if "$(WINRT)" != "yes"
SUBDIRS		= $(SUBDIRS) \
		  gc \
		  slicing \
		  faultTolerance \
		  checksum \
		  stringConverter \
		  background \
		  threadPoolPriority \
		  custom \
		  properties \
		  servantLocator \
		  defaultServant \
		  interceptor \
		  defaultValue
!endif


$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
