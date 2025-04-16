// Copyright (c) ZeroC, Inc.

#ifndef CRYPT_PLUGIN_FACTORY_H
#define CRYPT_PLUGIN_FACTORY_H

#include "Ice/Ice.h"

#ifndef CRYPT_PERMISSIONS_VERIFIER_API
#    ifdef CRYPT_PERMISSIONS_VERIFIER_API_EXPORTS
#        define CRYPT_PERMISSIONS_VERIFIER_API ICE_DECLSPEC_EXPORT
#    else
#        define CRYPT_PERMISSIONS_VERIFIER_API /**/
#    endif
#endif

#if defined(_MSC_VER) && !defined(CRYPT_PERMISSIONS_VERIFIER_API_EXPORTS) && !defined(ICE_DISABLE_PRAGMA_COMMENT)
// Automatically link with Glacier2CryptPermissionsVerifier[D].lib
#    pragma comment(lib, ICE_LIBNAME("Glacier2CryptPermissionsVerifier"))
#endif

namespace Glacier2CryptPermissionsVerifier
{
    CRYPT_PERMISSIONS_VERIFIER_API Ice::PluginFactory cryptPluginFactory();
}

#endif
