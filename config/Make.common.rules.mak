# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

SHELL			= /bin/sh
VERSION			= 3.3.0
SOVERSION		= 33

OBJEXT			= .obj

#
# Ensure ice_language has been set by the file that includes this one.
#
!if "$(ice_language)" == ""
!error ice_language must be defined
!endif

#
# The following variables might also be defined:
#
# - slice_translator: the name of the slice translator required for the build.
#   Setting this variable is required when building source trees other than the
#   the source distribution (e.g.: the demo sources).
#
# - ice_require_cpp: define this variable to check for the presence of the C++
#   dev kit and check for the existence of the include/Ice/Config.h header.
#

#
# First, check if we're building a source distribution. 
#
# If building from a source distribution, ice_dir is defined to the
# top-level directory of the source distribution and ice_cpp_dir is
# defined to the directory containing the C++ binaries and headers to
# use to build the sources.
#
!if exist ($(top_srcdir)\..\$(ice_language))

ice_dir = $(top_srcdir)\..
ice_src_dist = 1

#
# When building a source distribution, if ICE_HOME is specified, it takes precedence over 
# the source tree for building the language mappings. For example, this can be used to 
# build the Python language mapping using the translators from the distribution specified
# by ICE_HOME.
#
!if "$(ICE_HOME)" != ""

!if "$(slice_translator)" != ""
!if !exist ($(ICE_HOME)\bin\$(slice_translator))
!error Unable to find $(slice_translator) in $(ICE_HOME), please verify ICE_HOME is properly configured and Ice is correctly installed.
!endif
ice_cpp_dir = $(ICE_HOME)
!else
!message Ignoring ICE_HOME environment variable to build current source tree.
ice_cpp_dir = $(ice_dir)/cpp
!endif

!if exist ($(ice_dir)\cpp\bin\$(slice_translator))
!message Found $(slice_translator) in both ICE_HOME\bin and $(ice_dir)\cpp\bin, ICE_HOME\bin\$(slice_translator) will be used!
!endif

!else

ice_cpp_dir = $(ice_dir)\cpp

!endif

!endif

#
# Then, check if we're building against a binary distribution.
#
!if "$(ice_src_dist)" == ""

!if "$(slice_translator)" == ""
!error slice_translator must be defined
!endif

!if "$(ICE_HOME)" != ""
!if !exist ($(ICE_HOME)\bin\$(slice_translator))
!error Unable to find $(slice_translator) in $(ICE_HOME), please verify ICE_HOME is properly configured and Ice is correctly installed.
!endif
ice_dir = $(ICE_HOME)
!elseif exist ($(top_srcdir)/bin/$(slice_translator))
ice_dir = $(top_srcdir)
!elseif exist ("C:\Ice-$(VERSION)\bin\$(slice_translator)")
ice_dir = "C:\Ice-$(VERSION)"
!endif

!if "$(ice_dir)" == ""
!error Unable to find a valid Ice distribution, please verify ICE_HOME is properly configured and Ice is correctly installed.
!endif
ice_bin_dist = 1
!endif

#
# If ice_require_cpp is defined, ensure the C++ headers exist
#
!if "$(ice_require_cpp)" == "yes"
!if "$(ice_src_dist)" != ""
ice_cpp_header = $(ice_cpp_dir)\include\Ice\Config.h
!else
ice_cpp_header = $(ice_dir)\include\Ice\Config.h
!endif
!if !exist ($(ice_cpp_header))
!error Unable to find a valid Ice distribution with the C++ header files, please verify ICE_HOME is properly configured and Ice is correctly installed.
!endif
!endif

#
# Set slicedir to the path of the directory containing the Slice files.
#
slicedir		= $(ice_dir)\slice

