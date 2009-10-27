# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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

#OPTIMIZE		= yes

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
ice_language = vb
slice_translator = slice2cs.exe

!if exist ($(top_srcdir)\..\config\Make.common.rules.mak)
!include $(top_srcdir)\..\config\Make.common.rules.mak
!else
!include $(top_srcdir)\config\Make.common.rules.mak
!endif

!if "$(ice_src_dist)" != ""
csbindir		= $(ice_dir)\cs\bin
!else
csbindir		= $(ice_dir)\bin
!endif

#
# If a slice directory is contained along with this distribution -- use it. 
# Otherwise use paths relative to $(ICE_HOME).
#

VBC			= vbc -nologo /r:system.dll

VBCFLAGS = -warnaserror
!if "$(DEBUG)" == "yes"
VBCFLAGS 		= $(VBCFLAGS) -debug+ -define:DEBUG=yes
!endif

MCS			= csc -nologo

MCSFLAGS = -warnaserror -d:MAKEFILE_BUILD
!if "$(DEBUG)" == "yes"
!if "$(OPTIMIZE)" == "yes"
MCSFLAGS               = $(MCSFLAGS) -debug:pdbonly
!else
MCSFLAGS               = $(MCSFLAGS) -debug
!endif
MCSFLAGS               = $(MCSFLAGS) -define:DEBUG
!endif

!if "$(OPTIMIZE)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -optimize+
!endif

!if "$(ice_src_dist)" != ""
SLICE2CS		= "$(ice_cpp_dir)\bin\slice2cs.exe"
!else
SLICE2CS		= "$(ice_dir)\bin$(x64suffix)\slice2cs.exe"
!endif

EVERYTHING		= all clean depend config

.SUFFIXES:
.SUFFIXES:		.cs .vb .ice

.ice.cs:
	$(SLICE2CS) $(SLICE2CSFLAGS) $<

{$(SDIR)\}.ice{$(GDIR)}.cs:
	$(SLICE2CS) --output-dir $(GDIR) $(SLICE2CSFLAGS) $<

!if "$(SLICE_ASSEMBLY)" != ""
$(SLICE_ASSEMBLY): $(GEN_SRCS)
        $(MCS) $(MCSFLAGS) -target:library -out:$@ -r:$(csbindir)\Ice.dll $(GEN_SRCS)
!endif

all:: $(TARGETS) $(TARGETS_CONFIG) $(SLICE_ASSEMBLY)

clean::
	del /q $(TARGETS) $(TARGETS_CONFIG) $(SLICE_ASSEMBLY) *.pdb

config:: $(TARGETS_CONFIG)

!if "$(SLICE_SRCS)" != ""
depend::
	$(SLICE2CS) --depend $(SLICE2CSFLAGS) $(SLICE_SRCS) | python $(ice_dir)\config\makedepend.py > .depend
!else
depend::
!endif

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

!if "$(ice_src_dist)" != ""
$(TARGETS_CONFIG):
	@sn -q -p $(KEYFILE) tmp.pub && \
	sn -q -t tmp.pub > tmp.publicKeyToken && \
	set /P TMP_TOKEN= < tmp.publicKeyToken && \
        cmd /c "set PUBLIC_KEY_TOKEN=%TMP_TOKEN:~-16% && \
	del tmp.pub tmp.publicKeyToken && \
	nmake /nologo /f Makefile.mak config"
!else
$(TARGETS_CONFIG):
	@sn -q -T $(csbindir)\Ice.dll > tmp.publicKeyToken && \
	set /P TMP_TOKEN= < tmp.publicKeyToken && \
        cmd /c "set PUBLIC_KEY_TOKEN=%TMP_TOKEN:~-16% && \
	del tmp.pub tmp.publicKeyToken && \
	nmake /nologo /f Makefile.mak config"
!endif

!else

publicKeyToken = $(PUBLIC_KEY_TOKEN: =)
$(TARGETS_CONFIG):
        @echo "Generating" <<$@ "..."
<?xml version="1.0"?>
  <configuration>
    <runtime>
      <assemblyBinding xmlns="urn:schemas-microsoft-com:asm.v1">
        <dependentAssembly>
          <assemblyIdentity name="Glacier2" culture="neutral" publicKeyToken="$(publicKeyToken)"/>
          <codeBase version="$(INTVERSION).0" href="$(csbindir)\Glacier2.dll"/>
        </dependentAssembly>
        <dependentAssembly>
          <assemblyIdentity name="Ice" culture="neutral" publicKeyToken="$(publicKeyToken)"/>
          <codeBase version="$(INTVERSION).0" href="$(csbindir)\Ice.dll"/>
        </dependentAssembly>
        <dependentAssembly>
          <assemblyIdentity name="IcePatch2" culture="neutral" publicKeyToken="$(publicKeyToken)"/>
          <codeBase version="$(INTVERSION).0" href="$(csbindir)\IcePatch2.dll"/>
        </dependentAssembly>
        <dependentAssembly>
          <assemblyIdentity name="IceStorm" culture="neutral" publicKeyToken="$(publicKeyToken)"/>
          <codeBase version="$(INTVERSION).0" href="$(csbindir)\IceStorm.dll"/>
        </dependentAssembly>
        <dependentAssembly>
          <assemblyIdentity name="IceBox" culture="neutral" publicKeyToken="$(publicKeyToken)"/>
          <codeBase version="$(INTVERSION).0" href="$(csbindir)\IceBox.dll"/>
        </dependentAssembly>
        <dependentAssembly>
          <assemblyIdentity name="IceGrid" culture="neutral" publicKeyToken="$(publicKeyToken)"/>
          <codeBase version="$(INTVERSION).0" href="$(csbindir)\IceGrid.dll"/>
        </dependentAssembly>
        <dependentAssembly>
          <assemblyIdentity name="IceSSL" culture="neutral" publicKeyToken="$(publicKeyToken)"/>
          <codeBase version="$(INTVERSION).0" href="$(csbindir)\IceSSL.dll"/>
        </dependentAssembly>
	<qualifyAssembly partialName="IceSSL" fullName="IceSSL, Version=$(INTVERSION).0, Culture=neutral, PublicKeyToken=$(publicKeyToken)"/>
    </assemblyBinding>
  </runtime>
</configuration>
<<KEEP

!endif

!endif
