// Copyright (c) ZeroC, Inc.

#ifndef GLACIER2_NULL_PERMISSIONS_VERIFIER_H
#define GLACIER2_NULL_PERMISSIONS_VERIFIER_H

#include "Glacier2/Glacier2.h"

#include <string>
#include <vector>

namespace Glacier2Internal
{
    GLACIER2_API void
    setupNullPermissionsVerifier(const Ice::CommunicatorPtr&, const std::string&, const std::vector<std::string>&);
}

#endif
