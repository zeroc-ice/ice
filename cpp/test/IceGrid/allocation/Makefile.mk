# Copyright (c) ZeroC, Inc.

$(project)_programs = client server verifier

$(project)_client_dependencies = IceGrid Glacier2

$(project)_verifier_sources = PermissionsVerifier.cpp
$(project)_verifier_dependencies = Glacier2

tests += $(project)
