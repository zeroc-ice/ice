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

SUBDIRS		= acm \
		  adapterDeactivation \
		  admin \
		  ami \
		  binding \
		  dispatcher \
		  echo \
		  enums \
		  exceptions \
		  facets \
		  hash \
		  hold \
		  info \
		  inheritance \
		  invoke \
		  location \
		  metrics \
		  objects \
		  operations \
		  optional \
		  plugin \
		  proxy \
		  retry \
		  stream \
		  timeout \
		  udp

!if "$(WINRT)" != "yes"
SUBDIRS		= $(SUBDIRS) \
		  background \
		  custom \
		  defaultServant \
		  defaultValue \
		  faultTolerance \
		  gc \
		  interceptor \
		  logger \
		  networkProxy
		  properties \
		  servantLocator \
		  slicing \
		  stringConverter \
		  threadPoolPriority \
  		  checksum \
  		  defaultServant \
  		  interceptor \
!endif


$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
