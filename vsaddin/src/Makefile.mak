# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

!include $(top_srcdir)\config\Make.rules.mak

PRODUCT    	= IceVisualStudioAddin
PKG		= $(PRODUCT)-$(PKG_PREFIX)

INSTALL_SUBDIRS	= $(install_bindir)

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
		  IceVBConfigurationDialog.cs \
		  IceVBConfigurationDialog.Designer.cs \
		  OutputDirView.cs \
		  OutputDirView.Designer.cs \
		  Options.cs \
		  Util.cs

RESOURCES 	= /resource:IceCppConfigurationDialog.resx \
	   	  /resource:IceCsharpConfigurationDialog.resx \
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

!if "$(DISABLE_SYSTEM_INSTALL)" != "yes"
install::	
	@if not exist $(ADDIN_PREFIX) \
	    @echo "Creating $(ADDIN_PREFIX) ..." && \
	    mkdir $(ADDIN_PREFIX)
	copy ..\config\Ice-$(VS).AddIn $(ADDIN_PREFIX)
	cscript ..\config\fixinstalldir.vbs "$(prefix)\" "$(ADDIN_PREFIX)\Ice-$(VS).AddIn"

	@if exist "$(VSINSTALLDIR)\ItemTemplates\CSharp\1033" \
	copy ..\templates\Slice.zip "$(VSINSTALLDIR)\ItemTemplates\CSharp\1033\"

	@if not exist $(ALLUSERSPROFILE)\ZeroC \
	    @echo "Creating $(ALLUSERSPROFILE)\ZeroC ..." && \
	    mkdir $(ALLUSERSPROFILE)\ZeroC
	copy ..\config\Ice.props $(ALLUSERSPROFILE)\ZeroC\Ice.props

	@if exist "$(VSINSTALLDIR)\VC\vcprojectitems\" \
		@if not exist "$(VSINSTALLDIR)\VC\vcprojectitems\Slice" \
			@echo "Creating $(VSINSTALLDIR)\VC\vcprojectitems\Slice ..." && \
			mkdir "$(VSINSTALLDIR)\VC\vcprojectitems\Slice"
		copy ..\templates\vs\Slice\slice.vsdir "$(VSINSTALLDIR)\VC\vcprojectitems\Slice\" 
		copy ..\templates\vs\newslice.ice "$(VSINSTALLDIR)\VC\vcprojectitems\"
		copy ..\templates\vs\newslice.ico "$(VSINSTALLDIR)\VC\vcprojectitems\"
		copy ..\templates\vs\slice.vsdir "$(VSINSTALLDIR)\VC\vcprojectitems\"

	@if exist "$(VSINSTALLDIR)\VC#\CSharpProjectItems\" \
		@if not exist "$(VSINSTALLDIR)\VC#\CSharpProjectItems\Slice" \
			@echo "Creating $(VSINSTALLDIR)\VC#\CSharpProjectItems\Slice ..." && \
			mkdir "$(VSINSTALLDIR)\VC#\CSharpProjectItems\Slice"
		copy ..\templates\vs\Slice\slice.vsdir "$(VSINSTALLDIR)\VC#\CSharpProjectItems\Slice\" 
		copy ..\templates\vs\newslice.ice "$(VSINSTALLDIR)\VC#\CSharpProjectItems\"
		copy ..\templates\vs\newslice.ico "$(VSINSTALLDIR)\VC#\CSharpProjectItems\"
		copy ..\templates\vs\slice.vsdir "$(VSINSTALLDIR)\VC#\CSharpProjectItems\"

	@if exist "$(VSINSTALLDIR)\ItemTemplates\CSharp\Code\1033" \
		copy ..\templates\Slice.zip "$(VSINSTALLDIR)\ItemTemplates\CSharp\Code\1033\"
	@if exist "$(VSINSTALLDIR)\ItemTemplates\CSharp\Silverlight\1033" \
		copy ..\templates\Slice.zip "$(VSINSTALLDIR)\ItemTemplates\CSharp\Silverlight\1033\"
	@if exist "$(VSINSTALLDIR)\ItemTemplates\CSharp\Web\1033" \
		copy ..\templates\Slice.zip "$(VSINSTALLDIR)\ItemTemplates\CSharp\Web\1033\"
	devenv.exe /installvstemplates

	@echo Adding key "$(INSTALL_KEY)" in Windows registry
	@reg ADD "$(INSTALL_KEY)" /v InstallDir /d "$(prefix)" /f || \
	echo "Could not add registry keyword $(SDK_KEY)" && exit 1
!endif

clean::
	-del /q $(TARGETS) $(PDBS)

all::$(TARGETS)
