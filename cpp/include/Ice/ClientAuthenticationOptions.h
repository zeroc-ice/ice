//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_CLIENT_AUTHENTICATION_OPTIONS_H
#define ICE_CLIENT_AUTHENTICATION_OPTIONS_H

#include "SSLConnectionInfo.h"

#include <functional>

#if defined(_WIN32)
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
#elif defined(__APPLE__)
#    include <Security/SecureTransport.h>
#    include <Security/Security.h>
#endif

namespace Ice::SSL
{
    /**
     * The SSL configuration properties for client connections.
     */
    struct ClientAuthenticationOptions
    {
#if defined(_WIN32)
        /**
         * The credentials handler to configure SSL client connections on Windows. When set the SSL transport would
         * ignore all the IceSSL configuration properties and use the provided credentials handle.
         *
         * [See Schannel
         * Credentials](https://learn.microsoft.com/en-us/windows/win32/api/schannel/ns-schannel-sch_credentials).
         */
        CredHandle credentialsHandler;

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
            serverCertificateValidationCallback;
#elif defined(__APPLE__)
        // The client's certificate chain.
        CFArrayRef clientCeriticateChain;

        // A callback that allows selecting a certificate chain based on the target server's host name. When the
        // callback is set it has preference over a certificate chain set in clientCertificateChain.
        std::function<CFArrayRef(const std::string& host)> clientCertificateSelectionCallback;

        // The trusted root certificates. If set, the server's certificate chain is validated against these
        // certificates. If not set the system's root certificates are used.
        CFArrayRef trustedRootCertificates;

        // A callback that allows validating the server certificate chain. When set trustedRootCertificates are
        // not used.
        std::function<bool(SecTrustRef trust, const IceSSL::ConnectionInfoPtr& info)>
            serverCertificateValidationCallback;

        // A callback that is called before ssl handshake is started. The callback can be used to set additional SSL
        // contex parameters.
        std::function<void(SSLContextRef)> sslContextSetup;
#else
#endif
    };
}

#endif
