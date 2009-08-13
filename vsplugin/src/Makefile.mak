# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice for Silverlight  is licensed to you under the terms
# described in the LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

!include $(top_srcdir)\config\Make.rules.mak

PRODUCT    	= IceVisualStudioAddin
PKG		= $(PRODUCT)-$(PKG_PREFIX)

INSTALL_SUBDIRS	= $(install_bindir) $(install_configdir)

TARGETS		= $(top_srcdir)\bin\$(PKG).dll
PDBS		= $(top_srcdir)\bin\$(PKG).pdb

SRCS		=  AssemblyInfo.cs \
		   Builder.cs \
		   Connect.cs \
 		   FileTracker.cs \
		   IceCppConfigurationDialog.cs \
		   IceCppConfigurationDialog.Designer.cs \
		   IceCsharpConfigurationDialog.cs \
		   IceCsharpConfigurationDialog.Designer.cs \
		   IceSilverlightConfigurationDialog.cs \
		   IceSilverlightConfigurationDialog.Designer.cs \
		   Util.cs

RESOURCES 	= /resource:IceCppConfigurationDialog.resx \
	   	  /resource:IceCsharpConfigurationDialog.resx \
	   	  /resource:IceSilverlightConfigurationDialog.resx

MCSFLAGS	= $(MCSFLAGS) -out:$(TARGETS)

$(TARGETS):: $(SRCS)
	$(MCS) $(MCSFLAGS) $(RESOURCES) $(SRCS)

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
