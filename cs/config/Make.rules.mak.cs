# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
# Ice invokes unmanaged code to implement the following features:
#
# - Protocol compression
# - Signal processing in the Ice.Application class (Windows only)
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
# Define OPTIMIZE as yes if you want to build with optimization.
#

OPTIMIZE		= yes

#
# Set the key file used to sign assemblies.
#

!if "$(KEYFILE)" == ""
KEYFILE                 = $(top_srcdir)\..\config\IceDevKey.snk
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

bindir			= $(top_srcdir)\bin

install_bindir		= $(prefix)\bin
install_libdir		= $(prefix)\lib

!if "$(ice_src_dist)" != ""
refdir = $(bindir)
!else
refdir = $(ice_dir)\bin
!endif

MCS			= csc -nologo

MCSFLAGS = -warnaserror -d:MAKEFILE_BUILD
!if "$(DEBUG)" == "yes"
!if "$(OPTIMIZE)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -debug:pdbonly
!else
MCSFLAGS 		= $(MCSFLAGS) -debug
!endif
MCSFLAGS 		= $(MCSFLAGS) -define:DEBUG
!endif

!if "$(OPTIMIZE)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -optimize+
!endif

!if "$(ice_src_dist)" != ""
!if "$(ice_cpp_dir)" == "$(ice_dir)\cpp"
SLICE2CS		= "$(ice_cpp_dir)\bin\slice2cs.exe"
SLICEPARSERLIB		= "$(ice_cpp_dir)\lib\slice.lib"
!if !exist ($(SLICEPARSERLIB))
SLICEPARSERLIB		= "$(ice_cpp_dir)\lib\sliced.lib"
!endif
!else
SLICE2CS		= "$(ice_cpp_dir)\bin$(x64suffix)\slice2cs.exe"
SLICEPARSERLIB		= "$(ice_cpp_dir)\lib$(x64suffix)\slice.lib"
!if !exist ($(SLICEPARSERLIB))
SLICEPARSERLIB		= "$(ice_cpp_dir)\lib$(x64suffix)\sliced.lib"
!endif
!endif
!else
SLICE2CS		= "$(ice_dir)\bin$(x64suffix)\slice2cs.exe"
SLICEPARSERLIB		= "$(ice_dir)\lib$(x64suffix)\slice.lib"
!if !exist ($(SLICEPARSERLIB))
SLICEPARSERLIB		= "$(ice_dir)\lib$(x64suffix)\sliced.lib"
!endif
!endif

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.cs .ice

.ice.cs:
	$(SLICE2CS) $(SLICE2CSFLAGS) $<

{$(SDIR)\}.ice{$(GDIR)}.cs:
	$(SLICE2CS) --output-dir $(GDIR) $(SLICE2CSFLAGS) $<

all:: $(TARGETS)

AL      = al
POLICY  = policy.$(SHORT_VERSION).$(PKG)

!if "$(POLICY_TARGET)" != ""
all:: $(bindir)/$(POLICY_TARGET)
!endif

clean::
	del /q $(TARGETS) *.pdb

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

!if "$(POLICY_TARGET)" != ""

$(bindir)/$(POLICY_TARGET):
!if "$(PUBLIC_KEY_TOKEN)" == ""
!if "$(ice_src_dist)" != ""
	@sn -q -p $(KEYFILE) tmp.pub && \
	sn -q -t tmp.pub > tmp.publicKeyToken && \
	set /P TMP_TOKEN= < tmp.publicKeyToken && \
        cmd /c "set PUBLIC_KEY_TOKEN=%TMP_TOKEN:~-16% && \
	del tmp.pub tmp.publicKeyToken && \
	nmake /nologo /f Makefile.mak policy"
!else
	@sn -q -T $(ice_dir)\bin\Ice.dll > tmp.publicKeyToken && \
	set /P TMP_TOKEN= < tmp.publicKeyToken && \
        cmd /c "set PUBLIC_KEY_TOKEN=%TMP_TOKEN:~-16% && \
	del tmp.publicKeyToken && \
	nmake /nologo /f Makefile.mak policy"
!endif
!endif

publicKeyToken = $(PUBLIC_KEY_TOKEN: =)

policy:
        @echo <<$(POLICY)
<configuration>
  <runtime>
    <assemblyBinding xmlns="urn:schemas-microsoft-com:asm.v1">
      <dependentAssembly>
        <assemblyIdentity name="$(PKG)" publicKeyToken="$(publicKeyToken)" culture=""/>
        <publisherPolicy apply="yes"/>
        <bindingRedirect oldVersion="$(SHORT_VERSION).0.0" newVersion="$(VERSION).0"/>
      </dependentAssembly>
    </assemblyBinding>
  </runtime>
</configuration>
<<KEEP
	$(AL) /link:$(POLICY) /version:0.0.0.0 /out:$(POLICY_TARGET) /keyfile:$(KEYFILE)
	move $(POLICY) $(bindir)
	move $(POLICY_TARGET) $(bindir)

clean::
	del /q $(bindir)\$(POLICY) $(bindir)\$(POLICY_TARGET)

!endif

install::
