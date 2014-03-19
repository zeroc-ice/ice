# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Determines whether the extension uses PHP namespaces (requires
# PHP 5.3 or later).
#
!if "$(USE_NAMESPACES)" == ""
USE_NAMESPACES		= no
!endif

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Common definitions
#
ice_language     = php
slice_translator = slice2php.exe

!if exist ($(top_srcdir)\..\config\Make.common.rules.mak)
!include $(top_srcdir)\..\config\Make.common.rules.mak
!else
!include $(top_srcdir)\config\Make.common.rules.mak
!endif

!if exist ($(top_srcdir)\..\cpp\config\Make.rules.msvc)
!include $(top_srcdir)\..\cpp\config\Make.rules.msvc
!else
!include $(top_srcdir)\config\Make.rules.msvc
!endif

ICECPPFLAGS		= -I"$(slicedir)"
SLICE2PHPFLAGS		= $(ICECPPFLAGS)

!if "$(USE_NAMESPACES)" == "yes"
CPPFLAGS		= $(CPPFLAGS) -DICEPHP_USE_NAMESPACES
SLICE2PHPFLAGS		= $(SLICE2PHPFLAGS) -n
!endif

!if "$(ice_src_dist)" != ""
!if "$(ice_cpp_dir)" == "$(ice_dir)\cpp"
SLICE2PHP		= $(ice_cpp_dir)\bin\slice2php.exe
SLICEPARSERLIB		= $(ice_cpp_dir)\lib\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB		= $(ice_cpp_dir)\lib\sliced.lib
!endif
!else
SLICE2PHP		= $(ice_cpp_dir)\bin$(x64suffix)\slice2php.exe
SLICEPARSERLIB		= $(ice_cpp_dir)\lib$(x64suffix)\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB		= $(ice_cpp_dir)\lib$(x64suffix)\sliced.lib
!endif
!endif
!else
SLICE2PHP		= $(ice_dir)\bin$(x64suffix)\slice2php.exe
SLICEPARSERLIB		= $(ice_dir)\lib$(x64suffix)\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB		= $(ice_dir)\lib$(x64suffix)\sliced.lib
!endif
!endif

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.php

all:: $(SRCS) $(TARGETS)

clean::
	del /q $(TARGETS) *.obj *.bak

install::
