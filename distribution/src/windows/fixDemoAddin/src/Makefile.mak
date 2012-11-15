# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

!include $(top_srcdir)\config\Make.rules.mak

PRODUCT    	= IceFixDemoAddin
PKG		= $(PRODUCT)-$(PKG_PREFIX)

INSTALL_SUBDIRS	= $(install_bindir) $(install_configdir)

TARGETS		= $(top_srcdir)\bin\$(PKG).dll
PDBS		= $(top_srcdir)\bin\$(PKG).pdb

SRCS		= AssemblyInfo.cs \
		   Connect.cs

MCSFLAGS	= $(MCSFLAGS) -out:$(TARGETS)

$(TARGETS):: $(SRCS)
	$(MCS) $(MCSFLAGS) $(SRCS)

install::$(TARGETS)
	@if not exist $(prefix) \
	    @echo "Creating $(prefix) ..." && \
	    mkdir $(prefix)
	@for %i in ( $(INSTALL_SUBDIRS) ) do \
	    @if not exist %i \
		@echo "Creating %i ..." && \
		mkdir %i

	copy $(TARGETS) $(install_bindir)\$(PKG).dll
	copy ..\config\Ice-$(PKG_PREFIX).AddIn $(install_configdir)\Ice-$(PKG_PREFIX).AddIn
	
clean::
	-del /q $(TARGETS) $(PDBS)

all::$(TARGETS)
