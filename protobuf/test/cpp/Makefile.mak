# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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

COBJS		= Test.obj \
		  Client.obj \
		  AllTests.obj \
		  Test.pb.obj

SOBJS		= Test.obj \
		  TestI.obj \
		  Server.obj \
		  Test.pb.obj

SRCS		= $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

#
# Need to use -W0 to get rid of ugly warnings from generated protobuf file as
# well as prevent compile failure on x64 due to warnings from protobuf headers
#
CPPFLAGS	= $(ICE_CPPFLAGS) -I$(PROTOBUF_HOME)\include -I. -I../../../cpp/test/include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN #-W0
LIBS		= $(ICE_LDFLAGS) $(LIBS) /libpath:$(PROTOBUF_HOME)\lib libprotobuf$(LIBSUFFIX).lib

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

$(CLIENT): $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

# A generic rule would be nicer, but since protoc generates .pb.cpp
# it is not possible with nmake.
Test.pb.cpp: Test.proto
	del /q Test.pb.h Test.pb.cpp
	$(PROTOBUF_HOME)\bin\protoc --cpp_out=. Test.proto
	move Test.pb.cc Test.pb.cpp

Test.pb.h: Test.pb.cpp

clean::
	del /q Test.pb.h Test.pb.cpp
	del /q Test.cpp Test.h

!include .depend
