//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SERVER_AUTHENTICATION_OPTIONS_H
#define ICE_SERVER_AUTHENTICATION_OPTIONS_H

#include "SSLConnectionInfo.h"

#include <functional>

#ifdef _WIN32
// We need to include windows.h before wincrypt.h.
// clang-format off
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif
#    include <windows.h>
#    include <wincrypt.h>
// clang-format on
// SECURITY_WIN32 or SECURITY_KERNEL, must be defined before including security.h indicating who is compiling the code.
#    ifdef SECURITY_WIN32
#        undef SECURITY_WIN32
#    endif
#    ifdef SECURITY_KERNEL
#        undef SECURITY_KERNEL
#    endif
#    define SECURITY_WIN32 1
#    include <schannel.h>
#    include <security.h>
#    include <sspi.h>
#    undef SECURITY_WIN32
#endif

namespace Ice::SSL
{
    /**
     * The SSL configuration properties for client connections.
     */
    struct ServerAuthenticationOptions
    {
#if defined(_WIN32)
        /**
         * The credentials handler to configure SSL server connections on Windows. When set the SSL transport would
         * ignore all the IceSSL configuration properties and use the provided credentials handle.
         *
         * [See Schannel
         * Credentials](https://learn.microsoft.com/en-us/windows/win32/api/schannel/ns-schannel-sch_credentials).
         */
        CredHandle credentialsHandler;

        bool clientCertificateRequired;

        /**
         * A callback that allows to manually validate the server certificate during SSL handshake on Windows. When the
         * callback is not provided the server certificate will be validated using the platform default validation
         * mechanism.
         *
         * @param context A security context is an opaque data structure that contains security data relevant to the
         * current connection.
         * @return true if the certificate is valid, false otherwise.
         *
         * [See Manually Validating Schannel
         * Credentials](https://learn.microsoft.com/en-us/windows/win32/secauthn/manually-validating-schannel-credentials).
         */
        std::function<bool(CtxtHandle context, const IceSSL::ConnectionInfoPtr& info)>
            clientCertificateValidationCallback;
#elif defined(__APPLE__)
#else
#endif
    };
}

#endif
