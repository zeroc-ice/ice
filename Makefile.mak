# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir = cpp
!include cpp/config/Make.rules.mak


!if "$(CPP_COMPILER)" == "VC100"
SUBDIRS			= cpp python
CLEAN_SUBDIRS		= cpp python
DEPEND_SUBDIRS		= cpp python
INSTALL_SUBDIRS		= cpp python
TEST_SUBDIRS		= cpp python
!else
SUBDIRS			= cpp java js csharp
CLEAN_SUBDIRS		= cpp java js csharp
DEPEND_SUBDIRS		= cpp java js csharp
INSTALL_SUBDIRS		= cpp java csharp
TEST_SUBDIRS		= cpp java js csharp
!endif

!if "$(CPP_COMPILER)" == "VC110"
SUBDIRS			= $(SUBDIRS) php
CLEAN_SUBDIRS		= $(SUBDIRS) php
DEPEND_SUBDIRS		= $(SUBDIRS) php
INSTALL_SUBDIRS		= $(SUBDIRS) php
TEST_SUBDIRS		= $(SUBDIRS) php
!endif

all::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making all in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) all" || exit 1

clean::
	@for %i in ( $(CLEAN_SUBDIRS) ) do \
	    @echo "making clean in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) clean" || exit 1

depend::
	@for %i in ( $(DEPEND_SUBDIRS) ) do \
	    @echo "making depend in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) depend" || exit 1

install::
	@for %i in ( $(INSTALL_SUBDIRS) ) do \
	    @echo "making install in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) install" || exit 1

test::
	@for %i in ( $(TEST_SUBDIRS) ) do \
	    @echo "making test in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) test" || exit 1

cpp::
	@echo "making all in cpp" && \
	cmd /c "cd cpp && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) all" || exit 1

java::
	@echo "making all in java" && \
	cmd /c "cd java && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) all" || exit 1

csharp::
	@echo "making all in csharp" && \
	cmd /c "cd csharp && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) all" || exit 1

python::
	@echo "making all in python" && \
	cmd /c "cd python && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) all" || exit 1

php::
	@echo "making all in php" && \
	cmd /c "cd php && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) all" || exit 1

js::
	@echo "making all in js" && \
	cmd /c "cd js && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) all" || exit 1
