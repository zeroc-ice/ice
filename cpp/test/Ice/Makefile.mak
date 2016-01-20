# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

!include $(top_srcdir)\config\Make.rules.mak

!if "$(CPP11_MAPPING)" == "yes"
SUBDIRS		= proxy \
		  operations \
		  exceptions \
		  ami \
		  invoke \
		  info \
		  inheritance \
		  facets \
		  objects \
		  faultTolerance \
		  location \
		  adapterDeactivation \
		  slicing \
		  hash \
		  checksum \
		  dispatcher \
		  hold \
		  binding \
		  retry \
		  timeout \
		  acm \
		  servantLocator \
		  interceptor \
		  stringConverter \
		  background \
		  threadPoolPriority \
		  udp \
		  defaultServant \
		  defaultValue \
		  properties \
		  plugin \
		  admin \
		  metrics \
		  enums \
		  logger \
		  networkProxy \
		  services \
		  impl
!else
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
		  acm \
		  udp \
		  admin \
		  plugin \
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
		  defaultValue \
		  logger \
		  networkProxy \
		  properties \
		  servantLocator \
		  slicing \
		  stringConverter \
		  threadPoolPriority \
		  checksum \
		  defaultServant \
		  interceptor \
		  services \
		  impl
!endif
!endif

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
