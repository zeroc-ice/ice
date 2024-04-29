//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SERVER_AUTHENTICATION_OPTIONS_H
#define ICE_SERVER_AUTHENTICATION_OPTIONS_H

#include "SSLConfig.h"
#include "SSLConnectionInfo.h"

#include <functional>

namespace Ice::SSL
{
    /**
     * The SSL configuration properties for server connections.
     */
    struct ServerAuthenticationOptions
    {
#if defined(_WIN32)
        /**
         * A callback that allows selecting the server credentials based on the name of the object adapter that
         * accepts the connection.
         *
         * @param adapterName The name of the object adapter that accepted the connection.
         * @return The server credentials. The credentials must remain valid for the duration of the connection.
         *
         * [See Detailed Schannel documentation on Schannel credentials](
         * https://learn.microsoft.com/en-us/windows/win32/secauthn/acquirecredentialshandle--schannel)
         */
        std::function<SCHANNEL_CRED(const std::string& adapterName)> serverCredentialsSelectionCallback;

        /**
         * Whether or not the server requires that the client provides a certificate.
         */
        bool clientCertificateRequired;

        /**
         * A callback that allows manually validating the client certificate chain. When the verification callback
         * returns false, the connection will be aborted with an Ice::SecurityException.
         *
         * @param context A CtxtHandle representing the security context associated with the current connection. This
         * context contains security data relevant for validation, such as the client's certificate chain and cipher
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
            clientCertificateValidationCallback;

#elif defined(__APPLE__)
        /**
         * A callback that allows selecting the server's certificate chain based on the name of the object adapter that
         * accepts the connection.
         *
         * @param adapterName The name of the object adapter that accepted the connection.
         * @return The server's certificate chain. The certificate chain must remain valid for the duration of the
         * connection.
         *
         * The requirements for the Secure Transport certificates are documented in
         * https://developer.apple.com/documentation/security/1392400-sslsetcertificate?changes=_3&language=objc
         */
        std::function<CFArrayRef(const std::string& adapterName)> serverCertificateSelectionCallback;

        /**
         * The trusted root certificates. If set, the client's certificate chain is validated against these
         * certificates. If not set the system's root certificates are used.
         */
        CFArrayRef trustedRootCertificates;

        /**
         * The requirements for client-side authentication
         * @see https://developer.apple.com/documentation/security/sslauthenticate
         */
        SSLAuthenticate clientCertificateRequired;

        /**
         * A callback that is invoked before initiating a new SSL handshake. This callback provides an opportunity to
         * customize the SSL parameters for the session based on specific server settings or requirements.
         *
         * @param context An opaque type that represents an SSL session context object.
         * @param adapterName The name of the object adapter that accepted the connection.
         */
        std::function<void(SSLContextRef context, const std::string& adapterName)> sslNewSessionCallback;

        /**
         * A callback that allows manually validating the client certificate chain. When the verification callback
         * returns false, the connection will be aborted with an Ice::SecurityException.
         *
         * @param trust The trust object that contains the client's certificate chain.
         * @param info The IceSSL::ConnectionInfoPtr object that provides additional connection-related data which might
         * be relevant for contextual certificate validation.
         * @return true if the certificate chain is valid and the connection should proceed; false if the certificate
         * chain is invalid and the connection should be aborted. An exception may be thrown to indicate a custom
         * error during the validation process.
         */
        std::function<bool(SecTrustRef trust, const IceSSL::ConnectionInfoPtr& info)>
            clientCertificateValidationCallback;
#else
        /**
         * A callback that allows selecting the server's SSL context based on the server's adapter name. This callback
         * is used to associate a specific SSL configuration with a server instance, identified by the adapter name.
         *
         * @param adapterName The name of the object adapter that accepted the connection.
         * @return A pointer to an SSL_CTX object that represents the SSL configuration for the specified object
         * adapter.
         *
         * @see Detailed OpenSSL documentation on SSL_CTX management:
         * https://www.openssl.org/docs/manmaster/man3/SSL_CTX_new.html
         */
        std::function<SSL_CTX*(const std::string& adapterName)> serverSslContextSelectionCallback;

        /**
         * A callback that is invoked before initiating a new SSL handshake. This callback provides an opportunity to
         * customize the SSL parameters for the session based on specific server settings or requirements.
         *
         * @param ssl A pointer to the SSL object representing the connection.
         * @param adapterName The name of the object adapter that accepted the connection.
         *
         * @see Detailed OpenSSL documentation on SSL object management:
         * https://www.openssl.org/docs/manmaster/man3/SSL_new.html
         */
        std::function<void(::SSL* ssl, const std::string& adapterName)> sslNewSessionCallback;

        /**
         * A callback that allows manually validating the client certificate chain. When the verification callback
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
            clientCertificateValidationCallback;

#endif
    };
}

#endif
