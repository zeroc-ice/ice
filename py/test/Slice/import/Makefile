# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

include $(top_srcdir)/config/Make.rules

#
# Parallel builds are not supported because multiple executions of slice2py could
# attempt to modify the __init__.py file simultaneously.
#
.NOTPARALLEL:

SRCS	= Test1_ice.py \
          Test2_ice.py

all:: $(SRCS)

Test1_ice.py: Test1.ice $(SLICE2PY) $(SLICEPARSERLIB)
	$(SLICE2PY) $(SLICE2PYFLAGS) $<

Test2_ice.py: Test2.ice $(SLICE2PY) $(SLICEPARSERLIB)
	$(SLICE2PY) $(SLICE2PYFLAGS) $<

clean::
	rm -rf $(SRCS) Test

include .depend
