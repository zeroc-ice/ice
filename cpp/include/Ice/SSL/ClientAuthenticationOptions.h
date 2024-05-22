//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SSL_CLIENT_AUTHENTICATION_OPTIONS_H
#define ICE_SSL_CLIENT_AUTHENTICATION_OPTIONS_H

#include "Config.h"
#include "ConnectionInfo.h"

#include <functional>

namespace Ice::SSL
{
#if defined(ICE_USE_SCHANNEL)
    /**
     * The SSL configuration properties for client connections.
     */
    struct SchannelClientAuthenticationOptions
    {
        /**
         * A callback that allows selecting the client's SSL credentials based on the target server host name.
         *
         * @remarks This callback is invoked by the SSL transport for each new outgoing connection before starting the
         * SSL handshake to determine the appropriate client credentials. The callback should return a SCH_CREDENTIALS
         * that represents the client's credentials. The SSL transport takes ownership of the credentials' paCred and
         * and hRootStore, and releases them when the connection is closed.
         *
         * @param host The target server host name.
         * @return The client's credentials.
         *
         * Example of setting clientCertificateSelectionCallback:
         * ```cpp
         * PCCERT_CONTEXT _clientCertificate  = ...; //
         *
         * auto initData = Ice::InitializationData {
         *   ...
         *   .clientAuthenticationOptions = ClientAuthenticationOptions {
         *      .clientCredentialsSelectionCallback = [this](const std::string&)
         *      {
         *        // Increment the certificate context reference count to ensure it remains
         *        // valid for the duration of the connection. The SSL transport will release
         *        // it after closing the connection.
         *        CertDuplicateCertificateContext(_clientCertificate);
         *        SCH_CREDENTIALS credentials = _clientCredentials;
         *        credentials.cCreds = 1;
         *        credentials.paCred = &_clientCertificate;
         *        return credentials;
         *      }
         * };
         *
         * auto communicator = Ice::initialize(initData);
         * ...
         * CertFreeCertificateContext(_clientCertificate); // Release the certificate when no longer needed
         * ```
         *
         * See Detailed Wincrypt documentation for [SCH_CREDENTIALS](
         * https://learn.microsoft.com/en-us/windows/win32/api/schannel/ns-schannel-sch_credentials)
         */
        std::function<SCH_CREDENTIALS(const std::string& host)> clientCredentialsSelectionCallback;

        /**
         * A callback that is invoked before initiating a new SSL handshake. This callback provides an opportunity to
         * customize the SSL parameters for the session based on specific client settings or requirements.
         *
         * Example of setting sslNewSessionCallback:
         * ```cpp
         * auto initData = Ice::InitializationData {
         *   ...
         *   .clientAuthenticationOptions = ClientAuthenticationOptions {
         *     .sslNewSessionCallback =
         *       [](CtxtHandle context, const std::string& host) {
         *         SecPkgContext_ConnectionInfo connInfo;
         *         connInfo.dwProtocol = SP_PROT_TLS1_3;
         *         SECURITY_STATUS status = SetContextAttributes(
         *           &ctxHandle,
         *           SECPKG_ATTR_CONNECTION_INFO,
         *           &connInfo,
         *           sizeof(connInfo));
         *
         *         if (status != SEC_E_OK) {
         *           // Handle error
         *           ...
         *         }
         *       }
         *     }
         * };
         * auto communicator = Ice::initialize(initData);
         * ```
         *
         * @param context The CtxtHandle representing the security context associated with the current connection.
         * @param host The target server host name.
         */
        std::function<void(CtxtHandle context, const std::string& host)> sslNewSessionCallback;

        /**
         * The trusted root certificates used for validating the server's certificate chain. If this field is set, the
         * server's certificate chain is validated against these certificates; otherwise, the system's default root
         * certificates are used.
         *
         * @remarks The trusted root certificates are only used by the default validation callback; they are ignored by
         * custom validation callbacks set with serverCertificateValidationCallback.
         *
         * Example of setting trustedRootCertificates:
         * ```cpp
         * HCERTSTORE rootCerts = ...; // Populate with X.509 certificates
         *
         * auto initData = Ice::InitializationData {
         *   ...
         *   .clientAuthenticationOptions = ClientAuthenticationOptions {
         *      .trustedRootCertificates = rootCerts;
         *   }
         * };
         *
         * auto communicator = Ice::initialize(initData);
         * CertCloseStore(rootCerts); // It is safe to close the rootCerts store now.
         * ```
         */
        HCERTSTORE trustedRootCertificates = nullptr;

        /**
         * A callback that allows manually validating the server certificate chain. When the verification callback
         * returns false, the connection will be aborted with an Ice::SecurityException.
         *
         * @remarks The server certificate chain is validated using the context object passed to the callback. When this
         * callback is set, it replaces the default validation callback and must perform all necessary validation
         * steps. If trustedRootCertificates is set, the passed context object will use them as the trusted root
         * certificates for validation. This setting can be modified by the application using
         * [SecTrustSetAnchorCertificates](https://developer.apple.com/documentation/security/1396098-sectrustsetanchorcertificates?language=objc)
         *
         * Example of setting serverCertificateValidationCallback:
         * ```cpp
         * auto initData = Ice::InitializationData {
         *   ...
         *   .clientAuthenticationOptions = ClientAuthenticationOptions{
         *     .serverCertificateValidationCallback =
         *       [](SecTrustRef trust, const ConnectionInfoPtr& info)
         *       {
         *          ...
         *          return SecTrustEvaluateWithError(trust, nullptr);
         *       }
         * });
         * auto communicator = Ice::initialize(initData);
         * ```
         *
         * @param context A CtxtHandle representing the security context associated with the current connection. This
         * context contains security data relevant for validation, such as the client's certificate chain and cipher
         * suite.
         * @param info The ConnectionInfoPtr object that provides additional connection-related data which might
         * be relevant for contextual certificate validation.
         * @return true if the certificate chain is valid and the connection should proceed; false if the certificate
         * chain is invalid and the connection should be aborted.
         * @throws Ice::SecurityException if the certificate chain is invalid and the connection should be aborted.
         *
         * [See
         * SecTrustEvaluateWithError](https://developer.apple.com/documentation/security/2980705-sectrustevaluatewitherror?language=objc)
         */
        std::function<bool(CtxtHandle context, const ConnectionInfoPtr& info)> serverCertificateValidationCallback;
    };
    // Alias for portable code
    using ClientAuthenticationOptions = SchannelClientAuthenticationOptions;
#elif defined(ICE_USE_SECURE_TRANSPORT)
    /**
     * The SSL configuration properties for client connections.
     */
    struct SecureTransportClientAuthenticationOptions
    {
        /**
         * A callback that allows selecting the client's SSL certificate chain based on the target server host name.
         *
         * @remarks This callback is invoked by the SSL transport for each new outgoing connection before starting the
         * SSL handshake to determine the appropriate client certificate chain. The callback should return a CFArrayRef
         * that represents the client's certificate chain, or nullptr if no certificate chain should be used for the
         * connection. The SSL transport takes ownership of the returned CFArrayRef and releases it when the connection
         * is closed.
         *
         * @param host The target server host name.
         * @return CFArrayRef containing the client's certificate chain, or nullptr to indicate that no certificate is
         * used.
         *
         * Example of setting clientCertificateSelectionCallback:
         * ```cpp
         * _clientCertificateChain = CFArrayCreate(...); // Populate with X.509 certificate/key pair
         *
         * auto initData = Ice::InitializationData {
         *   ...
         *   .clientAuthenticationOptions = ClientAuthenticationOptions {
         *      .clientCertificateSelectionCallback = [this](const std::string&)
         *      {
         *        // Retain the client certificate chain to ensure it remains valid for the duration
         *        // of the connection. The SSL transport will release it after closing the connection.
         *        CFRetain(_clientCertificateChain);
         *        return _clientCertificateChain;
         *      }
         * };
         *
         * auto communicator = Ice::initialize(initData);
         * ...
         * CFRelease(_clientCertificateChain); // Release the CFArrayRef when no longer needed
         * ```
         *
         * See the Secure Transport documentation for requirements on the certificate chain format:
         * [SSLSetCertificate](https://developer.apple.com/documentation/security/1392400-sslsetcertificate?changes=_3&language=objc)
         */
        std::function<CFArrayRef(const std::string& host)> clientCertificateSelectionCallback;

        /**
         * The trusted root certificates used for validating the server's certificate chain. If this field is set, the
         * server's certificate chain is validated against these certificates; otherwise, the system's default root
         * certificates are used.
         *
         * @remarks The trusted root certificates are used by both the default validation callback, and by custom
         * validation callback set in serverCertificateValidationCallback.
         *
         * This is equivalent to calling
         * [SecTrustSetAnchorCertificates](https://developer.apple.com/documentation/security/1396098-sectrustsetanchorcertificates?language=objc)
         * with the CFArrayRef object; and
         * [SecTrustSetAnchorCertificatesOnly](https://developer.apple.com/documentation/security/1399071-sectrustsetanchorcertificatesonl?language=objc)
         * with the `anchorCertificatesOnly` parameter set to true.
         *
         * Example of setting trustedRootCertificates:
         * ```cpp
         * CFArrayRef rootCerts = CFArrayCreate(...); // Populate with X.509 certificates
         *
         * auto initData = Ice::InitializationData {
         *   ...
         *   .clientAuthenticationOptions = ClientAuthenticationOptions {
         *      .trustedRootCertificates = rootCerts;
         *   }
         * };
         *
         * auto communicator = Ice::initialize(initData);
         * CFRelease(rootCerts); // It is safe to release the rootCerts now.
         * ```
         */
        CFArrayRef trustedRootCertificates = nullptr;

        /**
         * A callback that is invoked before initiating a new SSL handshake. This callback provides an opportunity to
         * customize the SSL parameters for the session based on specific client settings or requirements.
         *
         * Example of setting sslNewSessionCallback:
         * ```cpp
         * auto initData = Ice::InitializationData {
         *   ...
         *   .clientAuthenticationOptions = ClientAuthenticationOptions{
         *     .sslNewSessionCallback =
         *       [](SSLContextRef context, const std::string& host)
         *       {
         *         // Customize the ssl context using SecureTransport API.
         *         OSStatus status = SSLSetProtocolVersionMin(context, kTLSProtocol13);
         *         if(status != noErr)
         *         {
         *            // Handle error
         *         }
         *         ...
         *       }
         *     }
         * };
         * auto communicator = Ice::initialize(initData);
         * ```
         *
         * @param context An opaque type that represents an SSL session context object.
         * @param host The target server host name.
         */
        std::function<void(SSLContextRef context, const std::string& host)> sslNewSessionCallback;

        /**
         * A callback that allows manually validating the server certificate chain. When the verification callback
         * returns false, the connection will be aborted with an Ice::SecurityException.
         *
         * @remarks The server certificate chain is validated using the trust object passed to the callback. When this
         * callback is set, it replaces the default validation callback and must perform all necessary validation
         * steps. If trustedRootCertificates is set, the passed trust object will use them as the anchor certificates
         * for evaluating trust. This setting can be modified by the application using
         * [SecTrustSetAnchorCertificates](https://developer.apple.com/documentation/security/1396098-sectrustsetanchorcertificates?language=objc)
         *
         * Example of setting serverCertificateValidationCallback:
         * ```cpp
         * auto initData = Ice::InitializationData {
         *   ...
         *   .clientAuthenticationOptions = ClientAuthenticationOptions{
         *     .serverCertificateValidationCallback =
         *       [](SecTrustRef trust, const ConnectionInfoPtr& info)
         *       {
         *          ...
         *          return SecTrustEvaluateWithError(trust, nullptr);
         *       }
         * });
         * auto communicator = Ice::initialize(initData);
         * ```
         *
         * @param trust The trust object that contains the server's certificate chain.
         * @param info The ConnectionInfoPtr object that provides additional connection-related data which might
         * be relevant for contextual certificate validation.
         * @return true if the certificate chain is valid and the connection should proceed; false if the certificate
         * chain is invalid and the connection should be aborted.
         * @throws Ice::SecurityException if the certificate chain is invalid and the connection should be aborted.
         *
         * [See
         * SecTrustEvaluateWithError](https://developer.apple.com/documentation/security/2980705-sectrustevaluatewitherror?language=objc)
         */
        std::function<bool(SecTrustRef trust, const ConnectionInfoPtr& info)> serverCertificateValidationCallback;
    };
    // Alias for portable code
    using ClientAuthenticationOptions = SecureTransportClientAuthenticationOptions;
#else
    /**
     * The SSL configuration properties for client connections.
     */
    struct OpenSSLClientAuthenticationOptions
    {
        /**
         * A callback that allows selecting the client's SSL_CTX object based on the target host name.
         *
         * @remarks This callback is used to associate a specific SSL configuration with an outgoing connection
         * identified by the target host name. The callback must return a pointer to a valid SSL_CTX object which was
         * previously initialized using the OpenSSL API. The SSL transport takes ownership of the returned SSL_CTX
         * pointer and releases it after closing the connection.
         *
         * If the application does not provide a callback, the Ice SSL transport will use a SSL_CTX object created
         * with SSL_CTX_new() for the connection, which uses the default OpenSSL configuration.
         *
         * The SSL transport calls this callback for each new outgoing connection to obtain the SSL_CTX object before
         * it starts the SSL handshake.
         *
         * @param host The target host name.
         * @return A pointer to a SSL_CTX objet representing the SSL configuration for the new outgoing connection.
         *
         * Example of setting clientSSLContextSelectionCallback:
         * ```cpp
         * SSL_CTX* _sslContext = SSL_CTX_new(TLS_method());
         * ...
         * auto initData = Ice::InitializationData {
         *   ...
         *   .clientAuthenticationOptions = ClientAuthenticationOptions {
         *     .clientSSLContextSelectionCallback = [this](const std::string&) {
         *       // Ensure the SSL context remains valid for the lifetime of the connection.
         *       SSL_CTX_up_ref(_sslContext);
         *       return _sslContext;
         *     }
         *   }
         * };
         *
         * auto communicator = Ice::initialize(initData);
         * ...
         * SSL_CTX_free(_sslContext); // Release ssl context when no longer needed
         * ```
         *
         * @see Detailed OpenSSL documentation on SSL_CTX management:
         * https://www.openssl.org/docs/manmaster/man3/SSL_CTX_new.html
         */
        std::function<SSL_CTX*(const std::string& host)> clientSSLContextSelectionCallback;

        /**
         * A callback that is invoked before initiating a new SSL handshake. This callback provides an opportunity to
         * customize the SSL parameters for the connection.
         *
         * @param ssl A pointer to the SSL object representing the connection.
         * @param host The target server host name.
         *
         * Example of setting sslNewSessionCallback:
         * ```cpp
         * auto initData = Ice::InitializationData {
         *   ...
         *   .clientAuthenticationOptions = ClientAuthenticationOptions {
         *     .sslNewSessionCallback = [this](SSL* ssl, const std::string&) {
         *       if (!SSL_set_tlsext_host_name(ssl, host.c_str())) {
         *          // Handle error
         *       }
         *     }
         *   }
         * };
         *
         * auto communicator = Ice::initialize(initData);
         * ```
         *
         * @see Detailed OpenSSL documentation on SSL object management:
         * https://www.openssl.org/docs/manmaster/man3/SSL_new.html
         */
        std::function<void(::SSL* ssl, const std::string& host)> sslNewSessionCallback;

        /**
         * A callback that allows manually validating the server certificate chain. When the verification callback
         * returns false, the connection will be aborted with an Ice::SecurityException.
         *
         * @param verified A boolean indicating whether the preliminary certificate verification done by OpenSSL's
         * built-in mechanisms succeeded or failed. True if the preliminary checks passed, false otherwise.
         * @param ctx A pointer to an X509_STORE_CTX object, which contains the certificate chain to be verified.
         * @param info The ConnectionInfoPtr object that provides additional connection-related data
         * which might be relevant for contextual certificate validation.
         * @return true if the certificate chain is valid and the connection should proceed; false if the certificate
         * chain is invalid and the connection should be aborted.
         * @throws Ice::SecurityException if the certificate chain is invalid and the connection should be aborted.
         *
         * Example of setting serverCertificateValidationCallback:
         * ```cpp
         * auto initData = Ice::InitializationData {
         *   ...
         *   .clientAuthenticationOptions = ClientAuthenticationOptions {
         *     .serverCertificateValidationCallback =
         *     [this](bool verified, X509_STORE_CTX* ctx, const ConnectionInfoPtr& info) {
         *       ...
         *       return verified;
         *     }
         *   }
         * };
         * auto communicator = Ice::initialize(initData);
         * ```
         *
         * @see More details on certificate verification in OpenSSL:
         * https://www.openssl.org/docs/manmaster/man3/SSL_set_verify.html
         * @see More about X509_STORE_CTX management:
         * https://www.openssl.org/docs/manmaster/man3/X509_STORE_CTX_new.html
         */
        std::function<bool(bool verified, X509_STORE_CTX* ctx, const ConnectionInfoPtr& info)>
            serverCertificateValidationCallback;
    };
    // Alias for portable code
    using ClientAuthenticationOptions = OpenSSLClientAuthenticationOptions;
#endif

}

#endif
