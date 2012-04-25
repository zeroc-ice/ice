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

PRODUCT    	= IceVisualStudioAddin
PKG		= $(PRODUCT)-$(PKG_PREFIX)

INSTALL_SUBDIRS	= $(install_bindir) $(install_configdir)

TARGETS		= $(top_srcdir)\bin\$(PKG).dll
PDBS		= $(top_srcdir)\bin\$(PKG).pdb

SRCS		= IceDialog.cs \
		   ExtraCompilerOptionsView.Designer.cs \
		   ExtraCompilerOptionsView.cs \
		   IncludePathView.Designer.cs \
		   IncludePathView.cs \
		   AssemblyInfo.cs \
		   Builder.cs \
		   Connect.cs \
 		   FileTracker.cs \
		   IceCppConfigurationDialog.cs \
		   IceCppConfigurationDialog.Designer.cs \
		   IceCsharpConfigurationDialog.cs \
		   IceCsharpConfigurationDialog.Designer.cs \
		   IceSilverlightConfigurationDialog.cs \
		   IceSilverlightConfigurationDialog.Designer.cs \
		   IceVBConfigurationDialog.cs \
		   IceVBConfigurationDialog.Designer.cs \
		   OutputDirView.cs \
		   OutputDirView.Designer.cs \
		   Options.cs \
		   Util.cs

RESOURCES 	= /resource:IceCppConfigurationDialog.resx \
	   	  /resource:IceCsharpConfigurationDialog.resx \
	   	  /resource:IceSilverlightConfigurationDialog.resx \
	   	  /resource:IceVBConfigurationDialog.resx \
		  /resource:ExtraCompilerOptionsView.resx \
		  /resource:IncludePathView.resx \
		  /resource:OutputDirView.resx

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
	copy ..\config\$(PROPERTY_SHEET) $(install_configdir)\$(PROPERTY_SHEET)
	
clean::
	-del /q $(TARGETS) $(PDBS)

all::$(TARGETS)
