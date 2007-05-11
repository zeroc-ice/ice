# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

CLIENT		= client.exe
SERVER		= server.exe

TARGETS		= $(CLIENT) $(SERVER)

OBJS		= Throughput.obj

!include $(top_srcdir)/config/Make.rules.mak

!if "$(EMBEDDED_DEVICE)" != ""

COBJS           = WinCEClient.obj

SOBJS           = WinCEServer.obj

!else

COBJS           = Client.obj

SOBJS           = Server.obj

!endif

SOBJS		= $(SOBJS) \
		  ThroughputI.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp)

CPPFLAGS	= -I. $(CPPFLAGS) -WX -DWIN32_LEAN_AND_MEAN

!if "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

$(CLIENT): $(OBJS) $(COBJS)
	$(LINK) $(LDFLAGS) $(CPDBFLAGS) ThroughputC.obj $(COBJS) /out:$@ $(MINLIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest

$(SERVER): $(OBJS) $(SOBJS)
	$(LINK) $(LDFLAGS) $(SPDBFLAGS) $(OBJS) $(SOBJS) /out:$@ $(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest

Client.obj: Client.cpp
        $(CXX) /c -DICEE_PURE_CLIENT $(CPPFLAGS) $(CXXFLAGS) Client.cpp

ThroughputC.obj Throughput.obj: Throughput.cpp
        $(CXX) /c $(CPPFLAGS) $(CXXFLAGS) Throughput.cpp
        $(CXX) /c -DICEE_PURE_CLIENT /FoThroughputC.obj $(CPPFLAGS) $(CXXFLAGS) Throughput.cpp

clean::
	del /q Throughput.cpp Throughput.h


