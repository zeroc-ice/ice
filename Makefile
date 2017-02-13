# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

SUBDIRS		= cpp java js python ruby php
CLEAN_SUBDIRS	= js java python ruby php cpp
DEPEND_SUBDIRS	= cpp python ruby php
INSTALL_SUBDIRS	= cpp java python ruby php

ifeq ($(shell uname),Darwin)
SUBDIRS		+= objective-c
CLEAN_SUBDIRS	+= objective-c
DEPEND_SUBDIRS	+= objective-c
INSTALL_SUBDIRS	+= objective-c
endif

all::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making all in $$subdir"; \
	    ( cd $$subdir && $(MAKE) all ) || exit 1; \
	done

clean::
	@for subdir in $(CLEAN_SUBDIRS); \
	do \
	    echo "making clean in $$subdir"; \
	    ( cd $$subdir && $(MAKE) clean ) || exit 1; \
	done

depend::
	@for subdir in $(DEPEND_SUBDIRS); \
	do \
	    echo "making depend in $$subdir"; \
	    ( cd $$subdir && $(MAKE) depend ) || exit 1; \
	done

install::
	@for subdir in $(INSTALL_SUBDIRS); \
	do \
	    echo "making install in $$subdir"; \
	    ( cd $$subdir && $(MAKE) install ) || exit 1; \
	done

test::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making test in $$subdir"; \
	    ( cd $$subdir && $(MAKE) test ) || exit 1; \
	done

cpp::
	echo "making all in cpp";
	( cd cpp && $(MAKE) all ) || exit 1;

java::
	echo "making all in java";
	( cd java && $(MAKE) all ) || exit 1;

cs::
	echo "making all in cs";
	( cd cs && $(MAKE) all ) || exit 1;

python::
	echo "making all in python";
	( cd python && $(MAKE) all ) || exit 1;

ruby::
	echo "making all in ruby";
	( cd ruby && $(MAKE) all ) || exit 1;

php::
	echo "making all in php";
	( cd php && $(MAKE) all ) || exit 1;
