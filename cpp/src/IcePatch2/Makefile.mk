# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_programs             := icepatch2server icepatch2client icepatch2calc
$(project)_dependencies         := IcePatch2 Ice
$(project)_targetdir            := $(bindir)

icepatch2server_sources         := $(addprefix $(currentdir)/,FileServerI.cpp Server.cpp)
icepatch2client_sources         := $(addprefix $(currentdir)/,Client.cpp)
icepatch2calc_sources           := $(addprefix $(currentdir)/,Calc.cpp)

projects += $(project)
