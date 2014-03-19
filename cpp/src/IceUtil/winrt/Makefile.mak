# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..
LIBNAME		= $(SDK_LIBRARY_PATH)\iceutil.lib
TARGETS		= $(LIBNAME)
SOURCE_DIR	= ..

TARGETS		= $(LIBNAME)

OBJS		= $(ARCH)\$(CONFIG)\ArgVector.obj \
                  $(ARCH)\$(CONFIG)\Cond.obj \
                  $(ARCH)\$(CONFIG)\ConvertUTF.obj \
		  $(ARCH)\$(CONFIG)\CountDownLatch.obj \
		  $(ARCH)\$(CONFIG)\Exception.obj \
		  $(ARCH)\$(CONFIG)\FileUtil.obj \
		  $(ARCH)\$(CONFIG)\InputUtil.obj \
		  $(ARCH)\$(CONFIG)\Options.obj \
		  $(ARCH)\$(CONFIG)\OutputUtil.obj \
		  $(ARCH)\$(CONFIG)\Random.obj \
		  $(ARCH)\$(CONFIG)\RecMutex.obj \
		  $(ARCH)\$(CONFIG)\Shared.obj \
		  $(ARCH)\$(CONFIG)\StringUtil.obj \
		  $(ARCH)\$(CONFIG)\Thread.obj \
		  $(ARCH)\$(CONFIG)\ThreadException.obj \
		  $(ARCH)\$(CONFIG)\Time.obj \
		  $(ARCH)\$(CONFIG)\Timer.obj \
		  $(ARCH)\$(CONFIG)\UUID.obj \
		  $(ARCH)\$(CONFIG)\Unicode.obj \
		  $(ARCH)\$(CONFIG)\MutexProtocol.obj

SRCS		= $(OBJS:.obj=.cpp)
SRCS		= $(SRCS:x86\=)
SRCS		= $(SRCS:x64\=)
SRCS		= $(SRCS:arm\=)
SRCS		= $(SRCS:Retail\=..\)
SRCS		= $(SRCS:Debug\=..\)

PDBNAME		= $(LIBNAME:.lib=.pdb)
CPPFLAGS        = /Fd$(PDBNAME) $(CPPFLAGS) -DICE_UTIL_API_EXPORTS -I..\.. -DWIN32_LEAN_AND_MEAN

!include $(top_srcdir)/config/Make.rules.mak

depend::
	del /q .depend.mak

.cpp.depend:
	$(CXX) /Fo$(ARCH)\$(CONFIG)\ /Zs /showIncludes $(CXXFLAGS) $(CPPFLAGS) $< 2>&1 | python.exe $(ice_dir)/config/makedepend-winrt.py $<

depend:: $(ARCH)\$(CONFIG) $(SRCS_DEPEND)

$(LIBNAME): $(OBJS) sdks
	$(AR) $(ARFLAGS) $(OBJS) /out:$(LIBNAME)

clean::
	-del /q $(ARCH)\$(CONFIG)\*.obj
	-del /q $(PDBNAME)
	
install:: all

!include .depend.mak
