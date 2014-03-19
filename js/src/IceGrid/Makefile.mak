# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME = IceGrid

GEN_SRCS = Admin.js \
	   Descriptor.js \
	   Exception.js \
	   FileParser.js \
	   Locator.js \
	   Observer.js \
	   Query.js \
	   Registry.js \
	   Session.js \
	   UserAccountMapper.js

SDIR		= $(slicedir)\IceGrid

SRCS		= $(GEN_SRCS)
INSTALL_SRCS	= IceGrid.js $(GEN_SRCS)

!include $(top_srcdir)\config\Make.rules.mak.js

SLICE2JSFLAGS	= $(SLICE2JSFLAGS) --ice -I"$(slicedir)"

MODULEDIR       = $(install_moduledir)\$(LIBNAME)

install:: all
        @if not exist $(MODULEDIR) \
            @echo "Creating $(MODULEDIR)" && \
            mkdir "$(MODULEDIR)"
        @for %i in ( $(INSTALL_SRCS) ) do \
            copy %i "$(MODULEDIR)"
        copy package.json "$(MODULEDIR)"
