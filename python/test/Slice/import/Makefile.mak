# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

!include $(top_srcdir)\config\Make.rules.mak

SRCS		= Test1_ice.py \
                  Test2_ice.py

!if "$(ice_src_dist)" != ""
SLICE2PY        = $(PYTHON_HOME)\python $(top_srcdir)\config\slice2py.py
!endif

all:: $(SRCS)

Test1_ice.py: "Test1.ice" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) "Test1.ice"

Test2_ice.py: "Test2.ice" "$(SLICEPARSERLIB)"
	"$(SLICE2PY)" $(SLICE2PYFLAGS) "Test2.ice"

clean::
	-rmdir /s /q Test
	del /q $(SRCS)

include .depend.mak
