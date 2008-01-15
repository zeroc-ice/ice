# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

SUBDIRS			= cpp java cs py rb php
DEPEND_SUBDIRS		= cpp cs py rb php
INSTALL_SUBDIRS		= cpp cs py rb php

all clean::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
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
