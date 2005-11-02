# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

TARGETS		= $(bindir)/iceboxnet.exe $(bindir)/iceboxcs.dll

L_SRCS		= AssemblyInfo.cs
I_SRCS		= Server.cs ServiceManagerI.cs

SLICE_SRCS	= $(SDIR)/IceBox.ice

SDIR		= $(slicedir)/IceBox
GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) --checksum --ice -I. -I$(slicedir)

$(bindir)/iceboxnet.exe: $(I_SRCS) $(bindir)/iceboxcs.dll
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(bindir)/iceboxcs.dll -r:$(bindir)/icecs.dll $(I_SRCS)

$(bindir)/iceboxcs.dll: $(L_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -target:library -out:$(bindir)/iceboxcs.dll -unsafe -r:$(bindir)/glacier2cs.dll -r:$(bindir)/icecs.dll $^

ifeq ($(NOGAC),)

install:: all
	$(call installprogram,$(bindir)/iceboxcs.dll,$(install_bindir))
	cd $(install_bindir) && $(GACUTIL) -i iceboxcs.dll && cd -
	$(call installprogram,$(bindir)/iceboxnet.exe,$(install_bindir))

else

install:: all
	$(call installprogram,$(bindir)/iceboxcs.dll,$(install_bindir))
	$(call installprogram,$(bindir)/iceboxnet.exe,$(install_bindir))

endif


include .depend
