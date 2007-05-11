# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\testcommon$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\testcommon$(SOVERSION)$(LIBSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

OBJS  		= TestCommon.obj

SRCS		= $(OBJS:.obj=.cpp)
		
!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I..\include $(CPPFLAGS) -DICE_TEST_COMMON_API_EXPORTS

!if "$(STATICLIBS)" != "yes" && "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

!if "$(EMBEDDED_DEVICE)" != ""
LIBS		= $(LIBS) 
!endif

!if "$(STATICLIBS)" == "yes"

$(DLLNAME):

$(LIBNAME): $(OBJS)
        $(AR) $(ARFLAGS) $(PDBFLAGS) $(OBJS) /out:$(LIBNAME)

!else

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	$(LINK) $(LDFLAGS) /dll $(PDBFLAGS) $(OBJS) /out:$(DLLNAME) $(LIBS)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

!endif

clean::
	del /q $(LIBNAME:.dll=.*)

!if "$(STATICLIBS)" != "yes"

clean::
	del /q $(DLLNAME:.dll=.*)

!endif

!include .depend
