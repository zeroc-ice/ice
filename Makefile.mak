# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir = cpp
!include cpp/config/Make.rules.mak

SUBDIRS			= cpp java py php
CLEAN_SUBDIRS		= java py php cpp
DEPEND_SUBDIRS		= cpp py php
INSTALL_SUBDIRS		= cpp java py php

!if "$(CPP_COMPILER)" == "VC60"
SUBDIRS			= $(SUBDIRS) rb
CLEAN_SUBDIRS		= rb $(CLEAN_SUBDIRS)
DEPEND_SUBDIRS		= $(DEPEND_SUBDIRS) rb
INSTALL_SUBDIRS		= $(INSTALL_SUBDIRS) rb
!else
SUBDIRS			= $(SUBDIRS) cs vb vsplugin
CLEAN_SUBDIRS		= cs vb vsplugin $(CLEAN_SUBDIRS)
DEPEND_SUBDIRS		= $(DEPEND_SUBDIRS) cs vb
INSTALL_SUBDIRS		= $(INSTALL_SUBDIRS) cs
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
	@for %i in ( $(INSTALL_SUBDIRS) ) do \
	    @echo "making install in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) install" || exit 1

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

rb::
	@echo "making all in rb" && \
	cmd /c "cd rb && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) all" || exit 1

php::
	@echo "making all in php" && \
	cmd /c "cd php && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) all" || exit 1

vsplugin::
	@echo "making all in vsplugin" && \
	cmd /c "cd vsplugin && $(MAKE) -nologo -f Makefile.mak $(MAKEFLAGS) all" || exit 1
