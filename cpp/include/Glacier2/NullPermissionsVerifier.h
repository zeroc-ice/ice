// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef GLACIER2_NULL_PERMISSIONS_VERIFIER_H
#define GLACIER2_NULL_PERMISSIONS_VERIFIER_H

#include <Ice/Config.h>
#include <Ice/CommunicatorF.h>
#include <string>
#include <vector>

#ifndef GLACIER2_API
#   if defined(ICE_STATIC_LIBS)
#       define GLACIER2_API /**/
#   elif defined(GLACIER2_API_EXPORTS)
#       define GLACIER2_API ICE_DECLSPEC_EXPORT
#   else
#       define GLACIER2_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace Glacier2Internal
{

GLACIER2_API void
setupNullPermissionsVerifier(const Ice::CommunicatorPtr&, const std::string&, const std::vector<std::string>&);

}

#endif
