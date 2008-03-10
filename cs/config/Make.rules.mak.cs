# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#

prefix			= C:\Ice-$(VERSION)

#
# The default behavior of 'nmake /f Makefile.mak install' attempts to add
# the Ice for .NET libraries to the Global Assembly Cache (GAC). If you would
# prefer not to install these libraries to the GAC, or if you do not have
# sufficient privileges to do so, then enable no_gac and the libraries will
# be copied to $(prefix)/bin instead.
#

#no_gac			= 1

#
# Ice invokes unmanaged code to implement the following features:
#
# - Protocol compression
# - Signal processing in the Ice.Application class (Windows only)
# - Monotonic time (Windows only)
#
# Enable MANAGED below if you do not require these features and prefer that
# the Ice run time use only managed code.
#
#MANAGED		= yes

#
# Define DEBUG as yes if you want to build with debug information and
# assertions enabled.
#

DEBUG			= yes

#
# Define OPTIMIZE as yes if you want to build with optimized.
#

#OPTIMIZE		= yes

#
# Set the key file used to sign assemblies.
#
!if "$(KEYFILE)" == ""
KEYFILE			= $(top_srcdir)\config\IceDevKey.snk
!endif

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Common definitions
#
ice_language = cs
slice_translator = slice2cs.exe

!if exist ($(top_srcdir)\..\config\Make.common.rules.mak)
!include $(top_srcdir)\..\config\Make.common.rules.mak
!else
!include $(top_srcdir)\config\Make.common.rules.mak
!endif

!if "$(ice_src_dist)" != ""
bindir			= $(ice_dir)\cs\bin
!else
bindir			= $(ice_dir)\bin
!endif

install_bindir		= $(prefix)\bin
install_libdir		= $(prefix)\lib

!if "$(no_gac)" != ""
NOGAC			= $(no_gac)
!endif

GACUTIL			= gacutil

MCS			= csc -nologo

MCSFLAGS = -warnaserror -d:MAKEFILE_BUILD
!if "$(DEBUG)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -debug -define:DEBUG
!endif

!if "$(OPTIMIZE)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -optimize+
!endif

!if "$(ice_src_dist)" != ""
SLICE2CS		= "$(ice_cpp_dir)\bin\slice2cs.exe"
!else
SLICE2CS		= "$(ice_dir)\bin$(x64suffix)\slice2cs.exe"
!endif

EVERYTHING		= all clean install config

.SUFFIXES:
.SUFFIXES:		.cs .ice

.ice.cs:
	$(SLICE2CS) $(SLICE2CSFLAGS) $<

{$(SDIR)\}.ice{$(GDIR)}.cs:
	$(SLICE2CS) --output-dir $(GDIR) $(SLICE2CSFLAGS) $<

#
# The .exe.config files are only generated if we're not building the 
# source distribution.
#
!if "$(ice_src_dist)" == ""
all:: $(TARGETS) $(TARGETS_CONFIG)

clean::
	del /q $(TARGETS) $(TARGETS_CONFIG) *.pdb
!else
all:: $(TARGETS)

clean::
	del /q $(TARGETS) *.pdb
!endif

config:: $(TARGETS_CONFIG)

!if "$(GEN_SRCS)" != ""
clean::
	del /q $(GEN_SRCS)
!endif
!if "$(CGEN_SRCS)" != ""
clean::
	del /q $(CGEN_SRCS)
!endif
!if "$(SGEN_SRCS)" != ""
clean::
	del /q $(SGEN_SRCS)
!endif
!if "$(GEN_AMD_SRCS)" != ""
clean::
	del /q $(GEN_AMD_SRCS)
!endif
!if "$(SAMD_GEN_SRCS)" != ""
clean::
	del /q $(SAMD_GEN_SRCS)
!endif

install::

!if "$(TARGETS_CONFIG)" != ""
$(TARGETS_CONFIG): $(TARGETS_CONFIG:.exe.config=.exe)
        @echo "Generating" <<$@ "..."
<?xml version="1.0"?>
  <configuration>
    <runtime>
      <assemblyBinding xmlns="urn:schemas-microsoft-com:asm.v1">
        <dependentAssembly>
          <assemblyIdentity name="Glacier2" culture="neutral" publicKeyToken="1f998c50fec78381"/>
          <codeBase version="$(VERSION).0" href="$(bindir)\Glacier2.dll"/>
        </dependentAssembly>
        <dependentAssembly>
          <assemblyIdentity name="Ice" culture="neutral" publicKeyToken="1f998c50fec78381"/>
          <codeBase version="$(VERSION).0" href="$(bindir)\Ice.dll"/>
        </dependentAssembly>
        <dependentAssembly>
          <assemblyIdentity name="IcePatch2" culture="neutral" publicKeyToken="1f998c50fec78381"/>
          <codeBase version="$(VERSION).0" href="$(bindir)\IcePatch2.dll"/>
        </dependentAssembly>
        <dependentAssembly>
          <assemblyIdentity name="IceStorm" culture="neutral" publicKeyToken="1f998c50fec78381"/>
          <codeBase version="$(VERSION).0" href="$(bindir)\IceStorm.dll"/>
        </dependentAssembly>
        <dependentAssembly>
          <assemblyIdentity name="IceBox" culture="neutral" publicKeyToken="1f998c50fec78381"/>
          <codeBase version="$(VERSION).0" href="$(bindir)\IceBox.dll"/>
        </dependentAssembly>
        <dependentAssembly>
          <assemblyIdentity name="IceGrid" culture="neutral" publicKeyToken="1f998c50fec78381"/>
          <codeBase version="$(VERSION).0" href="$(bindir)\IceGrid.dll"/>
        </dependentAssembly>
        <dependentAssembly>
          <assemblyIdentity name="IceSSL" culture="neutral" publicKeyToken="1f998c50fec78381"/>
          <codeBase version="$(VERSION).0" href="$(bindir)\IceSSL.dll"/>
        </dependentAssembly>
    </assemblyBinding>
  </runtime>
</configuration>
<<KEEP
!endif

