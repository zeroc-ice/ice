# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules

all::
	python setup.py build
	python setup.py install --install-lib $(libdir)

install::
	python setup.py install --install-lib $(install_libdir)

clean::
	rm -rf build
