# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

!include $(top_srcdir)\config\Make.rules.mak

SRCS		= Test1_ice.py \
                  Test2_ice.py

all:: $(SRCS)

Test1_ice.py: "Test1.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) "Test1.ice"

Test2_ice.py: "Test2.ice" "$(SLICE2PY)" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) "Test2.ice"

clean::
	-rmdir /s /q Test
	del /q $(SRCS)

include .depend.mak
