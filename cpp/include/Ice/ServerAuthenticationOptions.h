//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SERVER_AUTHENTICATION_OPTIONS_H
#define ICE_SERVER_AUTHENTICATION_OPTIONS_H

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
#else
#    include <openssl/ssl.h>
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
         * The server credentials.
         *
         * [See Schannel
         * Credentials](https://learn.microsoft.com/en-us/windows/win32/api/schannel/ns-schannel-sch_credentials).
         */
        CredHandle serverCredentials;

        /**
         * A callback that allows selecting the server credentials based on the server's host name. When the
         * callback is set it has preference over the credentials set in serverCredentials.
         *
         * @param host The target server's host name.
         * @return The server credentials.
         */
        std::function<CredHandle(const std::string& host)> serverCredentialsSelectionCallback;

        /**
         * A value indicating whether or not the server sends a client certificate request to the client.
         */
        bool clientCertificateRequired;

        /**
         * A callback that allows to manually validate the client certificate. When the callback is not provided the
         * client certificate will be validated using the platform default validation mechanism.
         *
         * The validation callback should return true if the certificate chain is valid, false otherwise. Alternatively
         * an exception can be throw to indicate an error.
         *
         * @param context A security context is an opaque data structure that contains security data relevant to the
         * current connection.
         * @param info The connection information.
         * @return true if the certificate is valid, false otherwise.
         *
         * [See Manually Validating Schannel
         * Credentials](https://learn.microsoft.com/en-us/windows/win32/secauthn/manually-validating-schannel-credentials).
         */
        std::function<bool(CtxtHandle context, const IceSSL::ConnectionInfoPtr& info)>
            clientCertificateValidationCallback;
#elif defined(__APPLE__)
        /**
         * The server's certificate chain.
         */
        CFArrayRef serverCertificateChain;

        /**
         * A callback that allows selecting a certificate chain based on the server's host name. When the callback is
         * set it has preference over a certificate chain set in serverCertificateChain.
         *
         * @param host The server's host name.
         * @return The server's certificate chain.
         */
        std::function<CFArrayRef(const std::string& host)> serverCertificateSelectionCallback;

        /**
         * The trusted root certificates. If set, the client's certificate chain is validated against these
         * certificates. If not set the system's root certificates are used.
         */
        CFArrayRef trustedRootCertificates;

        /**
         * A callback that allows manually validating the client certificate chain.
         *
         * The validation callback should return true if the certificate chain is valid, false otherwise. Alternatively
         * an exception can be throw to indicate an error.
         *
         * @param trust The trust object that contains the client's certificate chain.
         * @param info The connection information.
         * @return true if the certificate chain is valid, false otherwise.
         */
        std::function<bool(SecTrustRef trust, const IceSSL::ConnectionInfoPtr& info)>
            clientCertificateValidationCallback;

        /**
         * the requirements for client-side authentication
         * @see https://developer.apple.com/documentation/security/sslauthenticate
         */
        SSLAuthenticate clientCertificateRequired;

        /**
         * A callback that is called before a new SSL handshake starts. The callback can be used to set additional SSL
         * context parameters.
         */
        std::function<void(SSLContextRef)> sslNewSessionCallback;
#else
        /**
         * The server's SSL context, this objects holds configuration relevant the SSL session establishment.
         */
        SSL_CTX* serverSslContext;

        /**
         * A callback that is called before a new SSL handshake starts. The callback can be used to set additional SSL
         * parameters.
         */
        std::function<void(::SSL* ssl, const std::string& host)> sslNewSessionCallback;

        /**
         * A callback that allows manually validating the client certificate chain.
         *
         * The validation callback should return true if the certificate chain is valid, false otherwise. Alternatively
         * an exception can be throw to indicate an error.
         *
         * @param ok Whether the verification of the certificate in question was passed (ok=1) or not (ok=0).
         * @param ctx The X509_STORE_CTX object that holds the certificate chain to be verified.
         * @param info The connection information.
         * @return true if the certificate chain is valid, false otherwise.
         */
        std::function<int(int ok, X509_STORE_CTX*, const IceSSL::ConnectionInfoPtr& info)>
            clientCertificateVerificationCallback;
#endif
    };
}

#endif
