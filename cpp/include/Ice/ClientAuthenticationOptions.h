//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_CLIENT_AUTHENTICATION_OPTIONS_H
#define ICE_CLIENT_AUTHENTICATION_OPTIONS_H

#include "SSLConfig.h"
#include "SSLConnectionInfo.h"

#include <functional>

namespace Ice::SSL
{
    /**
     * The SSL configuration properties for client connections.
     */
    struct ClientAuthenticationOptions
    {
#if defined(_WIN32)
        /**
         * A callback that allows selecting the client's credentials based on the target server host name.
         *
         * @param host The target server host name.
         * @return The client credentials. The credentials must remain valid for the duration of the connection.
         *
         * [See Detailed Schannel documentation on Schannel credentials](
         * https://learn.microsoft.com/en-us/windows/win32/secauthn/acquirecredentialshandle--schannel)
         */
        std::function<SCHANNEL_CRED(std::string_view host)> clientCredentialsSelectionCallback;

        /**
         * A callback that allows manually validating the server certificate chain. When the verification callback
         * returns false, the connection will be aborted with an Ice::SecurityException.
         *
         * @param context A CtxtHandle representing the security context associated with the current connection. This
         * context contains security data relevant for validation, such as the server's certificate chain and cipher
         * suite.
         * @param info The IceSSL::ConnectionInfoPtr object that provides additional connection-related data
         * which might be relevant for contextual certificate validation.
         * @return true if the certificate chain is valid and the connection should proceed; false if the certificate
         * chain is invalid and the connection should be aborted. An exception may be thrown to indicate a custom
         * error during the validation process.
         *
         * [See Manually Validating Schannel Credentials for more
         * details](https://learn.microsoft.com/en-us/windows/win32/secauthn/manually-validating-schannel-credentials).
         */
        std::function<bool(CtxtHandle context, const IceSSL::ConnectionInfoPtr& info)>
            serverCertificateValidationCallback;
#elif defined(__APPLE__)
        /**
         * A callback that allows selecting the client's certificate chain based on the target server host name.
         *
         * @param host The target server host name.
         * @return The client's certificate chain. The certificate chain must remain valid for the duration of the
         * connection.
         *
         * The requirements for the Secure Transport certificate chain are documented in
         * https://developer.apple.com/documentation/security/1392400-sslsetcertificate?changes=_3&language=objc
         */
        std::function<CFArrayRef(std::string_view host)> clientCertificateSelectionCallback;

        /**
         * The trusted root certificates. If set, the server's certificate chain is validated against these
         * certificates. If not set the system's root certificates are used.
         */
        CFArrayRef trustedRootCertificates;

        /**
         * A callback that is invoked before initiating a new SSL handshake. This callback provides an opportunity to
         * customize the SSL parameters for the session.
         *
         * @param context An opaque type that represents an SSL session context object.
         * @param host The target server host name.
         */
        std::function<void(SSLContextRef context, std::string_view host)> sslNewSessionCallback;

        /**
         * A callback that allows manually validating the server certificate chain. When the verification callback
         * returns false, the connection will be aborted with an Ice::SecurityException.
         *
         * @param trust The trust object that contains the server's certificate chain.
         * @param info The IceSSL::ConnectionInfoPtr object that provides additional connection-related data which might
         * be relevant for contextual certificate validation.
         * @return true if the certificate chain is valid and the connection should proceed; false if the certificate
         * chain is invalid and the connection should be aborted. An exception may be thrown to indicate a custom
         * error during the validation process.
         */
        std::function<bool(SecTrustRef trust, const IceSSL::ConnectionInfoPtr& info)>
            serverCertificateValidationCallback;
#else
        /**
         * A callback that allows selecting the client's SSL context based on the target server host name. This callback
         * is used to associate a specific SSL configuration with a client connection instance, identified by the
         * target server host name.
         *
         * @param host The target server host name.
         * @return A pointer to an SSL_CTX object that represents the SSL configuration for the connection.
         *
         * @see Detailed OpenSSL documentation on SSL_CTX management:
         * https://www.openssl.org/docs/manmaster/man3/SSL_CTX_new.html
         */
        std::function<SSL_CTX*(std::string_view host)> clientSslContextSelectionCallback;

        /**
         * A callback that is invoked before initiating a new SSL handshake. This callback provides an opportunity to
         * customize the SSL parameters for the connection.
         *
         * @param ssl A pointer to the SSL object representing the connection.
         * @param host The target server host name.
         *
         * @see Detailed OpenSSL documentation on SSL object management:
         * https://www.openssl.org/docs/manmaster/man3/SSL_new.html
         */
        std::function<void(::SSL* ssl, std::string_view host)> sslNewSessionCallback;

        /**
         * A callback that allows manually validating the server certificate chain. When the verification callback
         * returns false, the connection will be aborted with an Ice::SecurityException.
         *
         * @param verified A boolean indicating whether the preliminary certificate verification done by OpenSSL's
         * built-in mechanisms succeeded or failed. True if the preliminary checks passed, false otherwise.
         * @param ctx A pointer to an X509_STORE_CTX object, which contains the certificate chain to be verified.
         * @param info The IceSSL::ConnectionInfoPtr object that provides additional connection-related data
         * which might be relevant for contextual certificate validation.
         * @return true if the certificate chain is valid and the connection should proceed; false if the certificate
         * chain is invalid and the connection should be aborted. An exception may be thrown to indicate a custom
         * error during the validation process.
         *
         * @note Throwing an exception from this callback will result in the termination of the connection.
         *
         * @see More details on certificate verification in OpenSSL:
         * https://www.openssl.org/docs/manmaster/man3/SSL_set_verify.html
         * @see More about X509_STORE_CTX management:
         * https://www.openssl.org/docs/manmaster/man3/X509_STORE_CTX_new.html
         */
        std::function<bool(bool verified, X509_STORE_CTX* ctx, const IceSSL::ConnectionInfoPtr& info)>
            serverCertificateValidationCallback;
#endif
    };
}

#endif
