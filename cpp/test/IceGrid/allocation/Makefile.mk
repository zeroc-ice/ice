# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_programs = client server verifier

$(test)_client_dependencies = IceGrid Glacier2

$(test)_verifier_sources = PermissionsVerifier.cpp
$(test)_verifier_dependencies = Glacier2

tests += $(test)
