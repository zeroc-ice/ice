# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

CA_FILES        = iceca \
                  iceca.bat 

CLASS_FILES     = ImportKey.class

TARGETS		= $(top_srcdir)\bin\iceca \
                  $(top_srcdir)\bin\iceca.bat \
		  $(top_srcdir)\lib\ImportKey.class

!include $(top_srcdir)\config\Make.rules.mak

$(top_srcdir)\bin\iceca: iceca
	copy iceca $@

$(top_srcdir)\bin\iceca.bat: iceca.bat
	copy iceca.bat $@

$(top_srcdir)\lib\ImportKey.class: ImportKey.class
	copy ImportKey.class $@

install::
	@for %i in ( $(CA_FILES) ) do \
	    @echo "Installing %i" && \
	    copy %i "$(prefix)\bin"
	@for %i in ( $(CLASS_FILES) ) do \
	    @echo "Installing %i" && \
	    copy %i "$(prefix)\lib"
