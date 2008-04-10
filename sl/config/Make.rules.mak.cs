# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Define DEBUG as yes if you want to build with debug information and
# assertions enabled.
#

#DEBUG			= yes

#
# Define OPTIMIZE as yes if you want to build with optimized.
#

OPTIMIZE		= yes


#
# Set the key file used to sign assemblies.
#

!if "$(KEYFILE)" == ""
KEYFILE			= $(top_srcdir)\..\config\IceDevKey.snk
!endif


# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Common definitions
#
ice_language = sl
slice_translator = slice2sl.exe

!if exist ($(top_srcdir)\..\config\Make.common.rules.mak)
!include $(top_srcdir)\..\config\Make.common.rules.mak
!else
!include $(top_srcdir)\config\Make.common.rules.mak
!endif

SILVERLIGHT		= yes

bindir			= $(top_srcdir)\bin

slicedir 		= $(top_srcdir)\slice

MCS			= csc -nologo

MCSFLAGS = -warnaserror -d:MAKEFILE_BUILD

!if "$(SILVERLIGHT)" == "yes"
MCSFLAGS = $(MCSFLAGS) -d:SILVERLIGHT /nowarn:618 /noconfig /nostdlib+ /reference:"C:\\Program Files\\Microsoft Silverlight\\agclr.dll" /reference:"C:\\Program Files\\Microsoft Silverlight\\mscorlib.dll" /reference:"C:\\Program Files\\Microsoft Silverlight\\System.Core.dll" /reference:"C:\\Program Files\\Microsoft Silverlight\\system.dll" /reference:"C:\\Program Files\\Microsoft Silverlight\\system.silverlight.dll" /reference:"C:\\Program Files\\Microsoft Silverlight\\system.Xml.core.dll"
!endif

!if "$(DEBUG)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -debug -define:DEBUG
!endif

!if "$(OPTIMIZE)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -optimize+
!endif

!if "$(ice_src_dist)" != ""
SLICE2SL		= "$(ice_cpp_dir)\bin\slice2sl.exe"
!else
SLICE2SL		= "$(ice_dir)\bin\slice2sl.exe"
!endif

# If the translator does not exist, then try to use the one from the
# PATH.
!if !exist ($(SLICE2SL))
SLICE2SL                = slice2sl.exe
!endif

EVERYTHING		= all clean config

.SUFFIXES:
.SUFFIXES:		.cs .ice

.ice.cs:
	$(SLICE2SL) $(SLICE2SLFLAGS) $<

{$(SDIR)\}.ice{$(GDIR)}.cs:
	$(SLICE2SL) --output-dir $(GDIR) $(SLICE2SLFLAGS) $<

all:: $(TARGETS) $(TARGETS_CONFIG)

clean::
	del /q $(TARGETS) $(TARGETS_CONFIG) *.pdb

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

!if "$(TARGETS_CONFIG)" != ""

!if "$(PUBLIC_KEY_TOKEN)" == ""

$(TARGETS_CONFIG):
        @sn -q -p $(KEYFILE) tmp.pub && \
        sn -q -t tmp.pub > tmp.publicKeyToken && \
        set /P TMP_TOKEN= < tmp.publicKeyToken && \
        cmd /c "set PUBLIC_KEY_TOKEN=%TMP_TOKEN:~-16% && \
        del tmp.pub tmp.publicKeyToken && \
        nmake /nologo /f Makefile.mak config"

!else

publicKeyToken = $(PUBLIC_KEY_TOKEN: =)
$(TARGETS_CONFIG):
        @echo "Generating" <<$@ "..."
<?xml version="1.0"?>
  <configuration>
    <runtime>
      <assemblyBinding xmlns="urn:schemas-microsoft-com:asm.v1">
        <dependentAssembly>
          <assemblyIdentity name="IceSL" culture="neutral" publicKeyToken="$(publicKeyToken)"/>
          <codeBase version="0.1.1.0" href="$(bindir)\IceSL.dll"/>
        </dependentAssembly>    </assemblyBinding>
  </runtime>
</configuration>
<<KEEP

!endif

!endif
