# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

$(test)_client_sources := $(notdir $(wildcard $(currentdir)/*.cpp))

tests += $(test)
