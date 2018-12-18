# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

$(project)_programs             := icepatch2server icepatch2client icepatch2calc
$(project)_dependencies         := IcePatch2 Ice
$(project)_targetdir            := $(bindir)

icepatch2server_sources         := $(addprefix $(currentdir)/,FileServerI.cpp Server.cpp)
icepatch2client_sources         := $(addprefix $(currentdir)/,Client.cpp)
icepatch2calc_sources           := $(addprefix $(currentdir)/,Calc.cpp)

projects += $(project)
