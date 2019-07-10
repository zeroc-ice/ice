#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(test)_ldflags      = $(iconv_ldflags)

$(test)_system_libs  = $(iconv_program_system_libs)

tests += $(test)
