# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

SUBDIRS			= cpp java cs py rb php
CLEAN_SUBDIRS		= java cs py rb php cpp
DEPEND_SUBDIRS		= cpp cs py rb php
INSTALL_SUBDIRS		= cpp java cs py rb php

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

py::
	echo "making all in py";
	( cd py && $(MAKE) all ) || exit 1;

rb::
	echo "making all in rb";
	( cd rb && $(MAKE) all ) || exit 1;

php::
	echo "making all in php";
	( cd php && $(MAKE) all ) || exit 1;
