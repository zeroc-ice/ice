# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

$(test)_programs = client server verifier

$(test)_client_dependencies     = IceGrid Glacier2

$(test)_server_dependencies     = Glacier2 IceSSL

$(test)_verifier_sources        = PermissionsVerifier.cpp Test.ice
$(test)_verifier_dependencies   = Glacier2

tests += $(test)
