#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(test)_programs = client server verifier

$(test)_client_dependencies = IceGrid Glacier2

$(test)_verifier_sources = PermissionsVerifier.cpp
$(test)_verifier_dependencies = Glacier2

tests += $(test)
