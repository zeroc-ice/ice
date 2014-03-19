# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
# Enable support for the .NET Compact Framework. This setting disables the
# following features:
#
# - Protocol compression
# - Signal processing in the Ice.Application class
# - Dynamic loading of Slice-generated class and exception factories
# - IceSSL
# - ICE_CONFIG environment variable
# - Dynamic loading of Slice checksums
# - Ice.TCP.SndSize and Ice.TCP.RcvSize
#

#COMPACT			= yes

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
# Enable support for Silverlight.This setting disables the following
# features:
#
# - Protocol compression
# - Ice.Application and Glacier2.Application classes
# - Dynamic loading of Slice-generated class and exception factories
# - IceSSL
# - ICE_CONFIG environment variable
# - Dynamic loading of Slice checksums
# - Thread priorities
# - Ice.StdOut and Ice.StdErr
# - Ice.PrintProcessId
# - Ice.LogFile
# - Load properties for Windows registry
#
#SILVERLIGHT		= yes

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

#FRAMEWORK = 3.5

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
assembliesdir   = $(top_srcdir)\Assemblies
!if "$(SILVERLIGHT)" == "yes"
assembliesdir   = $(assembliesdir)\sl
!endif

!if "$(COMPACT)" == "yes"
bindir			= $(top_srcdir)\bin\cf
assembliesdir   = $(assembliesdir)\cf
!endif

install_bindir          = $(prefix)\bin
install_assembliesdir   = $(prefix)\Assemblies
install_configdir   = $(prefix)\config

!if "$(COMPACT)" == "yes"
install_bindir          = $(install_bindir)\cf
install_assembliesdir   = $(install_assembliesdir)\cf
!endif

!if "$(SILVERLIGHT)" == "yes"
install_bindir          = $(install_bindir)\sl
install_assembliesdir   = $(install_assembliesdir)\sl
!endif

install_libdir		    = $(prefix)\lib

!if "$(ice_src_dist)" != ""
refdir = $(assembliesdir)
!else
refdir = $(ice_dir)\Assemblies

!if "$(COMPACT)" == "yes"
refdir    = $(refdir)\cf
!endif

!endif

!if "$(PATCH_VERSION)" != "0" && "$(PATCH_VERSION)" != "51"
generate_policies   = yes
!endif

MCS			= csc -nologo

#
# UNITY implies MANAGED.
#
!if "$(UNITY)" == "yes"
MANAGED			= yes
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

# Define for SupressMessage to work
#MCSFLAGS		= $(MCSFLAGS) -define:CODE_ANALYSIS

!if "$(FRAMEWORK)" == "3.5"
MCSFLAGS = $(MCSFLAGS) /noconfig /nostdlib+ /define:DOTNET3_5
MCSFLAGS = $(MCSFLAGS) /reference:"$(FRAMEWORKDIR)\v2.0.50727\mscorlib.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(FRAMEWORKDIR)\v2.0.50727\System.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(FRAMEWORKDIR)\v2.0.50727\System.Data.dll"
MCSFLAGS = $(MCSFLAGS) /reference:"$(PROGRAMFILES)\Reference Assemblies\Microsoft\Framework\v3.5\System.Core.dll"  
MCSFLAGS = $(MCSFLAGS) /reference:"$(PROGRAMFILES)\Reference Assemblies\Microsoft\Framework\v3.0\System.Runtime.Serialization.dll"
!elseif "$(COMPACT)" == "yes"
NETCF_HOME		= $(PROGRAMFILES)\Microsoft.NET\SDK\CompactFramework\v3.5\WindowsCE
NETCF_REFS		= "/r:$(NETCF_HOME)\mscorlib.dll" \
			  "/r:$(NETCF_HOME)\System.dll" \
			  "/r:$(NETCF_HOME)\System.Runtime.Serialization.dll"
MCSFLAGS 		= $(MCSFLAGS) -noconfig -nostdlib -define:COMPACT $(NETCF_REFS)
!elseif "$(UNITY)" == "yes"
#
# You can compile against the WebPlayer assemblies by enabling the MCSFLAGS line below.
# You'll need to change UNITY_LIBDIR to the appropriate directory for your system.
#
# This setting works on Windows XP:
#UNITY_LIBDIR		= C:\Documents and Settings\<user>\Local Settings\Application Data\Unity\WebPlayer\mono\Release3.x.x\Data\lib
# This setting works on Windows 7:
UNITY_LIBDIR		= C:\Users\<user>\AppData\LocalLow\Unity\WebPlayer\mono\Release3.x.x\Data\lib

UNITY_LIBS 		= "/r:$(UNITY_LIBDIR)\mscorlib.dll" \
			  "/r:$(UNITY_LIBDIR)\System.dll" \
			  "/r:$(UNITY_LIBDIR)\System.Core.dll"
#MCSFLAGS 		= $(MCSFLAGS) -noconfig -nostdlib $(UNITY_LIBS)
!elseif "$(SILVERLIGHT)" == "yes"

#
# Silverlight 5.0
#
!if "$(SILVERLIGHT_VERSION)" == ""
!if exist ("$(PROGRAMFILES)\Microsoft Silverlight\5.0.61118.0")
SILVERLIGHT_VERSION	= 5.0.61118.0
!endif
!endif

#
# Silverlight 5.1
#
!if "$(SILVERLIGHT_VERSION)" == ""
!if exist ("$(PROGRAMFILES)\Microsoft Silverlight\5.1.10411.0")
SILVERLIGHT_VERSION	= 5.1.10411.0
!endif
!endif

!if "$(SILVERLIGHT_VERSION)" == ""
!if exist ("$(PROGRAMFILES)\Microsoft Silverlight\5.1.20513.0")
SILVERLIGHT_VERSION	= 5.1.20513.0
!endif
!endif

SILVERLIGHT_HOME	= $(PROGRAMFILES)\Microsoft Silverlight\$(SILVERLIGHT_VERSION)

SILVERLIGHT_REFS		= "/r:$(SILVERLIGHT_HOME)\mscorlib.dll" \
					"/r:$(SILVERLIGHT_HOME)\System.Core.dll" \
					"/r:$(SILVERLIGHT_HOME)\system.dll" \
					"/r:$(SILVERLIGHT_HOME)\System.Windows.dll" \
					"/r:$(SILVERLIGHT_HOME)\System.Net.dll" \
					"/r:$(SILVERLIGHT_HOME)\System.Windows.Browser.dll"
MCSFLAGS 		= $(MCSFLAGS) -noconfig -nostdlib -define:SILVERLIGHT $(SILVERLIGHT_REFS)
bindir			= $(top_srcdir)\bin\sl
install_bindir		= $(prefix)\bin\sl
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
SLICE2CS		= $(ice_dir)\bin$(x64suffix)\slice2cs.exe
SLICEPARSERLIB		= $(ice_dir)\lib$(x64suffix)\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB		= $(ice_dir)\lib$(x64suffix)\sliced.lib
!endif
!endif

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.cs .ice

.ice.cs:
	"$(SLICE2CS)" $(SLICE2CSFLAGS) $<

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


#
# Registry keywords required by Visual Studio and Ice Visual Studio Add-in.
#
!if "$(PROCESSOR_ARCHITECTURE)" == "AMD64"
REGISTRY_PREFIX				= HKLM\Software\Wow6432Node
!else
REGISTRY_PREFIX				= HKLM\Software
!endif

DOTNET_ASSEMBLIES_KEY		= $(REGISTRY_PREFIX)\Microsoft\.NETFramework\v2.0.50727\AssemblyFoldersEx\Ice
POCKETPC_ASSEMBLIES_KEY		= $(REGISTRY_PREFIX)\Microsoft\.NETCompactFramework\v3.5.0.0\PocketPC\AssemblyFoldersEx\Ice
SMARTPHONE_ASSEMBLIES_KEY	= $(REGISTRY_PREFIX)\Microsoft\.NETCompactFramework\v3.5.0.0\Smartphone\AssemblyFoldersEx\Ice
WINDOWSCE_ASSEMBLIES_KEY	= $(REGISTRY_PREFIX)\Microsoft\.NETCompactFramework\v3.5.0.0\WindowsCE\AssemblyFoldersEx\Ice
SILVERLIGHT_ASSEMBLIES_KEY	= $(REGISTRY_PREFIX)\Microsoft\Microsoft SDKs\Silverlight\v5.0\AssemblyFoldersEx\Ice
