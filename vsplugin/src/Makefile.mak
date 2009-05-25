# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice for Silverlight  is licensed to you under the terms
# described in the ICESL_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

!include $(top_srcdir)\config\Make.rules.mak

PRODUCT    = IceVisualStudioAddin
PKG		= $(PRODUCT)-$(PKG_PREFIX)

INSTALL_SUBDIRS	= $(install_bindir) $(install_configdir)

TARGETS		= $(top_srcdir)\bin\$(PKG).dll
PDBS			= $(top_srcdir)\bin\$(PKG).pdb
SRCS		=  AssemblyInfo.cs \
		  Builder.cs \
		  Connect.cs \
                  CppProjectExtender.cs \
		  CSharpProjectExtender.cs \
		  Editors.cs \
		  ExtenderProvider.cs \
		  FileTracker.cs \
		  IncludeDirForm.cs \
		  IncludeDirForm.Designer.cs \
		  LibraryReferencesForm.cs \
		  LibraryReferencesForm.Designer.cs \
		  PreprocessorMacrosForm.cs \
		  PreprocessorMacrosForm.Designer.cs \
		  ProductReferencesForm.cs \
		  ProductReferencesForm.Designer.cs \
		  ProjectExtender.cs \
		  SilverlightProjectExtender.cs \
		  Util.cs

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
	copy ..\config\extender-$(PKG_PREFIX).reg $(install_configdir)\extender-$(PKG_PREFIX).reg
	
clean::
	del /q $(TARGETS) $(PDBS)

all::$(TARGETS)
