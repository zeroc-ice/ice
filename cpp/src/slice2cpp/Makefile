# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

NAME		= $(bindir)/slice2cpp$(EXE_EXT)

TARGETS		= $(NAME)

OBJS		= Gen.o \
		  Main.o

RPATH_DIR	= $(LOADER_PATH)/../$(libsubdir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I.. $(CPPFLAGS)
ifneq ($(findstring MINGW,$(UNAME)),)
LDEXEFLAGS 	:= -municode $(LDEXEFLAGS)
endif

$(NAME): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(OBJS) $(SLICE_LIBS) $(MCPP_RPATH_LINK)

install:: all
	$(call installprogram,$(NAME),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/slice2cpp.1,$(DESTDIR)$(install_mandir))
