# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#

!if "$(PREFIX)" == ""
prefix			= C:\Ice-$(VERSION)
!else
prefix			= $(PREFIX)
!endif

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
# Enable the UNITY flag to build for the Unity3D Web player. Enabling this
# setting also implies MANAGED. In addition to the features removed by
# MANAGED, this flag removes the following:
#
# - File I/O: property loading, file logger (Ice.LogFile), I/O redirection
#   (Ice.StdOut, Ice.StdErr)
# - ICE_CONFIG environment variable
#
#UNITY			= yes

#
# Define DEBUG as yes if you want to build with debug information and
# assertions enabled.
#

DEBUG			= yes

#
# Define OPTIMIZE as yes if you want to build with optimization.
#

#OPTIMIZE		= yes

#
# Define FRAMEWORK as 3.5 to force a .NET 3.5 build with Visual Studio 2010.
#

#FRAMEWORK 		= 3.5

#
# Set the key file used for strong name signing of assemblies.
#
!if "$(KEYFILE)" == ""
KEYFILE                 = $(top_srcdir)\..\config\IceDevKey.snk
!endif

#
# Define if you want the Ice assemblies to be authenticode signed.
#
#SIGN_CERTIFICATE	= MyCertFile.pfx
#SIGN_PASSWORD		= MyCertPassword
#SIGN_TIMESTAMPSERVER	= http://timestamp.verisign.com/scripts/timstamp.dll

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Common definitions
#
ice_language = csharp
slice_translator = slice2cs.exe

!if exist ($(top_srcdir)\..\config\Make.common.rules.mak)
!include $(top_srcdir)\..\config\Make.common.rules.mak
!else
!include $(top_srcdir)\config\Make.common.rules.mak
!endif

bindir			= $(top_srcdir)\bin
assembliesdir   = $(top_srcdir)\Assemblies

install_bindir          = $(prefix)\bin
install_assembliesdir   = $(prefix)\Assemblies
install_configdir   	= $(prefix)\config
install_libdir		= $(prefix)\lib

!if "$(ice_src_dist)" != ""
refdir = $(assembliesdir)
!else
refdir = $(ice_dir)\Assemblies
!endif

!if "$(PATCH_VERSION)" != "0" && "$(PATCH_VERSION)" != "51"
generate_policies   = yes
!endif

MCS = csc -nologo

#
# UNITY implies MANAGED.
#
!if "$(UNITY)" == "yes"
MANAGED	= yes
!endif

MCSFLAGS = -warnaserror

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

!if "$(MANAGED)" == "yes"
MCSFLAGS		= $(MCSFLAGS) -define:MANAGED
!endif

!if "$(UNITY)" == "yes"
MCSFLAGS		= $(MCSFLAGS) -define:UNITY
!endif

# Define for SupressMessage to work
#MCSFLAGS		= $(MCSFLAGS) -define:CODE_ANALYSIS

!if "$(FRAMEWORK)" == "3.5"
MCSFLAGS = $(MCSFLAGS) /noconfig /nostdlib+ /define:DOTNET3_5
MCSFLAGS = $(MCSFLAGS) /reference:"$(FRAMEWORKDIR)\v2.0.50727\mscorlib.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(FRAMEWORKDIR)\v2.0.50727\System.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(FRAMEWORKDIR)\v2.0.50727\System.Data.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(PROGRAMFILES)\Reference Assemblies\Microsoft\Framework\v3.5\System.Core.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(PROGRAMFILES)\Reference Assemblies\Microsoft\Framework\v3.0\System.Runtime.Serialization.dll"
!elseif "$(UNITY)" == "yes"
#
# You can compile against the WebPlayer assemblies by enabling the MCSFLAGS line below.
# You'll need to change UNITY_LIBDIR to the appropriate directory for your system.
#
UNITY_LIBDIR		= C:\Program Files\Unity\Editor\Data\Mono\lib\mono\unity_web
UNITY_LIBS 		= "/r:$(UNITY_LIBDIR)\mscorlib.dll" \
			  "/r:$(UNITY_LIBDIR)\System.dll" \
			  "/r:$(UNITY_LIBDIR)\System.Core.dll"
MCSFLAGS 		= $(MCSFLAGS) -noconfig -nostdlib $(UNITY_LIBS)
!endif

!if "$(ice_src_dist)" != ""
!if "$(ice_cpp_dir)" == "$(ice_dir)\cpp"
SLICE2CS		= $(ice_cpp_dir)\bin\slice2cs.exe
SLICEPARSERLIB		= $(ice_cpp_dir)\lib\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB		= $(ice_cpp_dir)\lib\sliced.lib
!endif
!else
SLICE2CS		= $(ice_cpp_dir)\bin$(x64suffix)\slice2cs.exe
SLICEPARSERLIB		= $(ice_cpp_dir)\lib$(x64suffix)\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB		= $(ice_cpp_dir)\lib$(x64suffix)\sliced.lib
!endif
!endif
!else
SLICE2CS		= $(ice_dir)\bin\slice2cs.exe
SLICEPARSERLIB		= $(ice_dir)\lib\slice.lib
!endif

EVERYTHING		= all clean install depend
EVERYTHING_EXCEPT_INSTALL = all clean depend

.SUFFIXES:
.SUFFIXES:		.cs .ice

DEPEND_DIR 	= .depend.mak

!if "$(GEN_SRCS)" != ""
$(GEN_SRCS): "$(SLICE2CS)" "$(SLICEPARSERLIB)"
GEN_DEPENDS = $(GEN_SRCS:.cs=.d)
GEN_DEPENDS = $(GEN_DEPENDS:generated\=.depend.mak\)
DEPENDS = $(DEPENDS) $(GEN_DEPENDS)
!endif

!if "$(CGEN_SRCS)" != ""
$(CGEN_SRCS): "$(SLICE2CS)" "$(SLICEPARSERLIB)"
CGEN_DEPENDS = $(CGEN_SRCS:.cs=.d)
CGEN_DEPENDS = $(CGEN_DEPENDS:generated\=.depend.mak\)
DEPENDS = $(DEPENDS) $(CGEN_DEPENDS)
!endif

!if "$(SGEN_SRCS)" != ""
$(SGEN_SRCS): "$(SLICE2CS)" "$(SLICEPARSERLIB)"
SGEN_DEPENDS = $(SGEN_SRCS:.cs=.d)
SGEN_DEPENDS = $(SGEN_DEPENDS:generated\=.depend.mak\)
DEPENDS = $(DEPENDS) $(SGEN_DEPENDS)
!endif

!if "$(GEN_AMD_SRCS)" != ""
$(GEN_AMD_SRCS): "$(SLICE2CS)" "$(SLICEPARSERLIB)"
GEN_AMD_DEPENDS = $(GEN_AMD_SRCS:.cs=.d)
GEN_AMD_DEPENDS = $(GEN_AMD_DEPENDS:generated\=.depend.mak\)
DEPENDS = $(DEPENDS) $(GEN_AMD_DEPENDS)
!endif

!if "$(SAMD_GEN_SRCS)" != ""
$(SAMD_GEN_SRCS): "$(SLICE2CS)" "$(SLICEPARSERLIB)"
SAMD_GEN_DEPENDS = $(SAMD_GEN_SRCS:.cs=.d)
SAMD_GEN_DEPENDS = $(SAMD_GEN_DEPENDS:generated\=.depend.mak\)
DEPENDS = $(DEPENDS) $(SAMD_GEN_DEPENDS)
!endif

!if exist(.depend.mak)
!include .depend.mak
!endif

depend::

!if "$(DEPENDS)" != ""
depend::
	@del /q .depend.mak

depend:: $(DEPENDS)
!endif

{$(SDIR)}.ice{$(DEPEND_DIR)}.d:
	@echo Generating dependencies for $<
	@"$(SLICE2CS)" --output-dir $(GDIR) $(SLICE2CSFLAGS) --depend $< |\
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-slice.vbs $(*F).ice

{$(SDIR)}.ice{$(GDIR)}.cs:
	"$(SLICE2CS)" --output-dir $(GDIR) $(SLICE2CSFLAGS) $<

all:: $(TARGETS)

AL      = al
POLICY  = policy.$(SHORT_VERSION).$(PKG)

!if "$(generate_policies)" == "yes" && "$(POLICY_TARGET)" != ""
all:: $(assembliesdir)/$(POLICY_TARGET)
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

!if "$(generate_policies)" == "yes" && "$(POLICY_TARGET)" != ""

$(assembliesdir)/$(POLICY_TARGET):
!if "$(PUBLIC_KEY_TOKEN)" == ""
!if "$(ice_src_dist)" != ""
	@sn -q -p "$(KEYFILE)" tmp.pub && \
	sn -q -t tmp.pub > tmp.publicKeyToken && \
	set /P TMP_TOKEN= < tmp.publicKeyToken && \
        cmd /c "set PUBLIC_KEY_TOKEN=%TMP_TOKEN:~-16% && \
	del tmp.pub tmp.publicKeyToken && \
	nmake /nologo /f Makefile.mak policy"
!else
	@sn -q -T $(ice_dir)\Assemblies\Ice.dll > tmp.publicKeyToken && \
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
        <bindingRedirect oldVersion="$(SHORT_VERSION).0.0 - $(SHORT_VERSION).1.0" newVersion="$(VERSION).0"/>
      </dependentAssembly>
    </assemblyBinding>
  </runtime>
</configuration>
<<KEEP
	$(AL) /link:$(POLICY) /version:0.0.0.0 /out:$(POLICY_TARGET) /keyfile:"$(KEYFILE)"
	move $(POLICY) $(assembliesdir)
	move $(POLICY_TARGET) $(assembliesdir)

clean::
	del /q $(assembliesdir)\$(POLICY) $(assembliesdir)\$(POLICY_TARGET)

!endif

install::
