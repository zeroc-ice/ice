# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

PKG		= IceBox
LIBNAME		= $(assembliesdir)\$(PKG).dll
ICEBOXNET	= $(bindir)\iceboxnet.exe
TARGETS		= $(LIBNAME) $(ICEBOXNET)
POLICY_TARGET   = $(POLICY).dll

L_SRCS		= AssemblyInfo.cs
I_SRCS		= AssemblyInfoExe.cs Server.cs ServiceManagerI.cs

GEN_SRCS	= $(GDIR)\IceBox.cs

SDIR		= $(slicedir)\IceBox
GDIR		= generated


!include $(top_srcdir)/config/Make.rules.mak.cs

all:: $(ICEBOXNET).config

EXE_MCSFLAGS	= $(MCSFLAGS) -target:exe

LIB_MCSFLAGS	= $(MCSFLAGS) -target:library -out:$(LIBNAME)
LIB_MCSFLAGS	= $(LIB_MCSFLAGS) -keyfile:"$(KEYFILE)"
LIB_MCSFLAGS	= $(LIB_MCSFLAGS) /doc:$(assembliesdir)\$(PKG).xml /nowarn:1591

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) --checksum --ice -I. -I$(slicedir)

$(ICEBOXNET): $(I_SRCS) $(LIBNAME)
	$(MCS) $(EXE_MCSFLAGS) -out:$@ -r:$(LIBNAME) -r:$(refdir)\Ice.dll $(I_SRCS)
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

$(LIBNAME): $(L_SRCS) $(GEN_SRCS)
	$(MCS) /baseaddress:0x25000000 $(LIB_MCSFLAGS) -r:$(refdir)\Ice.dll $(L_SRCS) $(GEN_SRCS)
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

!if "$(DEBUG)" == "yes"
clean::
	del /q $(assembliesdir)\$(PKG).pdb
	del /q $(bindir)\iceboxnet.pdb
!endif

clean::
	del /q $(assembliesdir)\$(PKG).xml

!if "$(PUBLIC_KEY_TOKEN)" == ""
$(ICEBOXNET).config:
	@sn -q -T $(assembliesdir)\Ice.dll > tmp.publicKeyToken && \
	set /P TMP_TOKEN= < tmp.publicKeyToken && \
        cmd /c "set PUBLIC_KEY_TOKEN=%TMP_TOKEN:~-16% && \
	del tmp.publicKeyToken && \
	nmake /nologo /f Makefile.mak iceboxnetconfig"
!endif

publicKeyToken = $(PUBLIC_KEY_TOKEN: =)

iceboxnetconfig:
        echo <<$(ICEBOXNET).config
<?xml version="1.0"?>
<configuration>
   <runtime>
      <assemblyBinding xmlns="urn:schemas-microsoft-com:asm.v1">
         <dependentAssembly>
            <assemblyIdentity name="Ice"  culture="neutral" publicKeyToken="$(publicKeyToken)"/>
            <codeBase version="3.6.3.0" href="..\Assemblies\Ice.dll"/>
         </dependentAssembly>
         <dependentAssembly>
            <assemblyIdentity name="IceBox"  culture="neutral" publicKeyToken="$(publicKeyToken)"/>
            <codeBase version="3.6.3.0" href="..\Assemblies\IceBox.dll"/>
         </dependentAssembly>
         <!-- 
           This allows iceboxnet to load the IceSSL plug-in using a strong name. We omit the
           optional attributes culture and publicKeyToken so they can be also omitted in the
	   IceSSL entry point.
	 -->
         <dependentAssembly>
            <assemblyIdentity name="IceSSL"/>
            <codeBase version="3.6.3.0" href="..\Assemblies\IceSSL.dll"/>
         </dependentAssembly>
      </assemblyBinding>
   </runtime>
</configuration>
<<KEEP

install:: all
	copy $(LIBNAME) "$(install_assembliesdir)"
	copy $(assembliesdir)\$(PKG).xml "$(install_assembliesdir)"
!if "$(generate_policies)" == "yes"
	copy $(assembliesdir)\$(POLICY_TARGET) "$(install_assembliesdir)"
!endif
!if "$(DEBUG)" == "yes"
	copy $(assembliesdir)\$(PKG).pdb "$(install_assembliesdir)"
!endif

install:: all
    copy $(ICEBOXNET) "$(install_bindir)"
    copy $(ICEBOXNET).config "$(install_bindir)"
!if "$(DEBUG)" == "yes"
	copy $(bindir)\iceboxnet.pdb "$(install_bindir)"
!endif

clean::
	del /q $(ICEBOXNET).config
