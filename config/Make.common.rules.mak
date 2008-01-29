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

# Ensure ice_language has been set by the file that includes this one.
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
# The following code below checks if:
#
# - we're building a source distribution (in which case either the translator from the
#   source distribution is used or the one from an external binary distribution specified
#   with ICE_HOME=<path> or -Dice.home=<path>).
#
# - we're building against a binary distribution (e.g.: building the demos using a binary
#   distribution specified with ICE_HOME=<path> or -Dice.home=<path>).
#
# NOTE: Changes made to these rules should also be applied to java/config/common.xml
#
# Once the kind of distribution is determined either ice_src_dist, ice_bin_dist
# is defined and the ice_dir variable contains the home of the Ice distribution.
# If we're building a source distribution, ice_cpp_dir is also set to the home of
# the Ice distribution containing the C++ binaries (which can be different from
# $(ice_dir)/cpp).
#

#
# First, check if we're building a source distribution.
#
!if exist ($(top_srcdir)\..\$(ice_language))
#
# When building a source distribution, we allow using either the translators
# from a binary distribution or the local translators.
#
ice_dir = $(top_srcdir)\..
!if exist ($(ice_dir)\cpp\bin\$(slice_translator))
ice_cpp_dir = $(ice_dir)\cpp
!else
!if "$(ICE_HOME)" != "" && exist ($(ICE_HOME)\bin\$(slice_translator))
ice_cpp_dir = $(ICE_HOME)
!else
ice_cpp_dir = $(ice_dir)\cpp
!endif
!endif
!endif
!if "$(ice_dir)" != ""
ice_src_dist = 1
!endif

#
# Then, check if we're building against a binary distribution.
#
!if "$(ice_dir)" == ""
!if "$(slice_translator)" == ""
!error slice_translator must be defined
!endif

!if exist ($(top_srcdir)\bin\$(slice_translator))
ice_dir = $(top_srcdir)
!else
!if "$(ICE_HOME)" != "" && exist ($(ICE_HOME)\bin\$(slice_translator))
ice_dir = $(ICE_HOME)
!endif
!endif
!if "$(ice_dir)" != ""
ice_bin_dist = 1
!endif
!endif

#
# At this point, either ice_src_dist or ice_bin_dist should be set, if not
# we couldn't find a valid Ice distribution.
#
!if "$(ice_dir)" == ""
!error Unable to find a valid Ice distribution, please verify ICE_HOME is properly configured and Ice is correctly installed.
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
