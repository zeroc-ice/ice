# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir = cpp
!include cpp/config/Make.rules.mak


!if "$(CPP_COMPILER)" == "VC100"
SUBDIRS			= cpp py
CLEAN_SUBDIRS		= py cpp
DEPEND_SUBDIRS		= cpp py
INSTALL_SUBDIRS		= cpp py
TEST_SUBDIRS		= cpp py
!else
SUBDIRS			= cpp java js cs vb vsaddin
CLEAN_SUBDIRS		= vsaddin vb cs js java cpp
DEPEND_SUBDIRS		= cpp java js cs vb
INSTALL_SUBDIRS		= cpp java js cs
TEST_SUBDIRS		= cpp java js cs
!endif

!if "$(CPP_COMPILER)" == "VC110"
SUBDIRS			= $(SUBDIRS) php
CLEAN_SUBDIRS		= php $(SUBDIRS)
DEPEND_SUBDIRS		= $(SUBDIRS) php
INSTALL_SUBDIRS		= $(SUBDIRS) php
TEST_SUBDIRS		= $(SUBDIRS) php
!endif

all::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making all in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) all" || exit 1

clean::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making clean in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) clean" || exit 1

depend::
	@for %i in ( $(DEPEND_SUBDIRS) ) do \
	    @echo "making depend in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) depend" || exit 1

install::
	@for %i in ( $(TEST_SUBDIRS) ) do \
	    @echo "making install in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) install" || exit 1

test::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making test in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) test" || exit 1

cpp::
	@echo "making all in cpp" && \
	cmd /c "cd cpp && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) all" || exit 1

java::
	@echo "making all in java" && \
	cmd /c "cd java && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) all" || exit 1

cs::
	@echo "making all in cs" && \
	cmd /c "cd cs && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) all" || exit 1

vb::
	@echo "making all in vb" && \
	cmd /c "cd vb && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) all" || exit 1

py::
	@echo "making all in py" && \
	cmd /c "cd py && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) all" || exit 1

php::
	@echo "making all in php" && \
	cmd /c "cd php && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) all" || exit 1

vsaddin::
	@echo "making all in vsaddin" && \
	cmd /c "cd vsaddin && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) all" || exit 1
