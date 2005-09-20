# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBNAME		= icegridcs.dll
TARGETS		= $(bindir)/$(LIBNAME)

SRCS		= AssemblyInfo.cs

SLICE_SRCS	= $(SDIR)/Admin.ice \
		  $(SDIR)/Observer.ice \
		  $(SDIR)/Descriptor.ice \
		  $(SDIR)/Exception.ice \
		  $(SDIR)/Query.ice

SDIR		= $(slicedir)/IceGrid
GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:library -out:$(TARGETS) -unsafe

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) --ice -I$(slicedir)

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -r:$(bindir)/glacier2cs.dll -r:$(bindir)/icecs.dll $^

ifeq ($(NOGAC),)

install:: all
	$(call installprogram,$(bindir)/$(LIBNAME),$(install_bindir))
	cd $(install_bindir) && $(GACUTIL) -i $(LIBNAME) && cd -

else

install:: all
	$(call installprogram,$(bindir)/$(LIBNAME),$(install_bindir))

endif

include .depend
