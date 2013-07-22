# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

install_dir = ${CURDIR}/mcpp

mcpp/lib/libmcpp.a:
	cd mcpp-2.7.2 && \
	./configure --prefix=$(install_dir) CFLAGS=-fPIC --enable-mcpplib --disable-shared && \
	make install DESTDIR=
	
all: mcpp/lib/libmcpp.a

install: all

clean:
	@if [ -f mcpp-2.7.2/Makefile ]; then \
		cd mcpp-2.7.2 && make distclean; \
	fi;
	@rm -rf mcpp