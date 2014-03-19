# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice Protobuf is licensed to you under the terms
# described in the ICE_PROTOBUF_LICENSE file included in this
# distribution.
#
# **********************************************************************

#
# If protobuf is not installed in a standard location where the
# compiler can find it, set PROTOBUF_HOME to the protobuf installation
# directory.
#
!if "$(PROTOBUF_HOME)" == ""
PROTOBUF_HOME   = c:\protobuf
!endif

top_srcdir	= ..\..

CLIENT		= client.exe
SERVER		= server.exe

TARGETS		= $(CLIENT) $(SERVER)

OBJS		= Hello.obj \
		  Person.pb.obj

COBJS		= Client.obj

SOBJS		= HelloI.obj \
		  Server.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

#
# Need to use -W0 to get rid of ugly warnings from generated protobuf file as
# well as prevent compile failure on x64 due to warnings from protobuf headers
#
CPPFLAGS	= $(ICE_CPPFLAGS) -I$(PROTOBUF_HOME)\include -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN -W0

LIBS		= $(ICE_LDFLAGS) $(LIBS) /libpath:$(PROTOBUF_HOME)\lib libprotobuf$(LIBSUFFIX).lib

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

$(CLIENT): $(OBJS) $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(OBJS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(OBJS) $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(OBJS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

# A generic rule would be nicer, but since protoc generates .pb.cpp
# it is not possible with nmake.
Person.pb.cpp: Person.proto
	del /q Person.pb.h Person.pb.cpp
	$(PROTOBUF_HOME)\bin\protoc --cpp_out=. Person.proto
	move Person.pb.cc Person.pb.cpp

Person.pb.h: Person.pb.cpp

clean::
	del /q Hello.cpp Hello.h
	del /q Person.pb.h Person.pb.cpp

!include .depend
