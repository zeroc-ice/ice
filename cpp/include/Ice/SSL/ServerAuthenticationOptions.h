// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_SERVER_AUTHENTICATION_OPTIONS_H
#define ICE_SSL_SERVER_AUTHENTICATION_OPTIONS_H

#include "Config.h"
#include "ConnectionInfo.h"

#include <functional>

// This file defines the `XxxServerAuthenticationOptions` structure for each platform-specific SSL implementation. The
// `#if defined(ICE_USE_XXX)/#endif` directives are used to include the appropriate structure based on the platform. We
// avoid using `#elif` directives because we want to define all the structures when building the doxygen documentation.

namespace Ice::SSL
{
#if defined(ICE_DOXYGEN)
    /// An alias for the platform-specific implementation of the SSL %ServerAuthenticationOptions.
    using ServerAuthenticationOptions = ...;
#endif

#if defined(ICE_USE_SCHANNEL) || defined(ICE_DOXYGEN)
    /// The SSL transport options for server connections on Windows.
    ///
    /// The SchannelServerAuthenticationOptions structure is only available when the Ice library is built on
    /// Windows. For macOS and iOS, see SecureTransportServerAuthenticationOptions, and for Linux, see
    /// OpenSSLServerAuthenticationOptions.
    /// @see ::Ice::SSL::ServerAuthenticationOptions
    struct SchannelServerAuthenticationOptions
    {
        /// A callback for selecting the server's SSL credentials based on the name of the object adapter that accepts
        /// the connection.
        ///
        /// This callback is invoked by the SSL transport for each new incoming connection before starting the SSL
        /// handshake to determine the appropriate server credentials. The callback must return a `SCH_CREDENTIALS` that
        /// represents the server's credentials. The SSL transport takes ownership of the credentials' `paCred` and
        /// `hRootStore` members and releases them when the connection is closed.
        ///
        /// @param adapterName The name of the object adapter that accepted the connection.
        /// @return The server SSL credentials.
        ///
        /// Example of setting `serverCertificateSelectionCallback`:
        /// @snippet Ice/SSL/SchannelServerAuthenticationOptions.cpp serverCertificateSelectionCallback
        ///
        /// @see [SCH_CREDENTIALS]
        ///
        /// [SCH_CREDENTIALS]:
        /// https://learn.microsoft.com/en-us/windows/win32/api/schannel/ns-schannel-sch_credentials
        std::function<SCH_CREDENTIALS(const std::string& adapterName)> serverCredentialsSelectionCallback;

        /// A callback invoked before initiating a new SSL handshake, providing an opportunity to customize the SSL
        /// parameters for the session based on specific server settings or requirements.
        ///
        /// @param context An opaque type that represents the security context associated with the current connection.
        /// @param adapterName The name of the object adapter that accepted the connection.
        std::function<void(CtxtHandle context, const std::string& adapterName)> sslNewSessionCallback;

        /// Whether or not the client must provide a certificate. The default value is false.
        bool clientCertificateRequired = false;

        /// The trusted root certificates used for validating the client's certificate chain. If this field is set, the
        /// client's certificate chain is validated against these certificates; otherwise, the system's default root
        /// certificates are used.
        ///
        /// Example of setting `trustedRootCertificates`:
        /// @snippet Ice/SSL/SchannelServerAuthenticationOptions.cpp trustedRootCertificates
        HCERTSTORE trustedRootCertificates = nullptr;

        /// A callback for validating the client certificate chain. If the verification callback returns false, the
        /// connection will be aborted with an Ice::SecurityException.
        ///
        /// When this callback is set, it replaces the default validation callback and must perform all necessary
        /// validation steps.
        ///
        /// Example of setting `clientCertificateValidationCallback`:
        /// @snippet Ice/SSL/SchannelServerAuthenticationOptions.cpp clientCertificateValidationCallback
        ///
        /// @param context An opaque object representing the security context associated with the current connection.
        /// This context contains security data relevant for validation, such as the server's certificate chain and
        /// cipher suite.
        /// @param info The connection info object that provides additional connection-related data. The
        /// `ConnectionInfo` type is an alias for the platform-specific connection info class.
        /// @return `true` if the certificate chain is valid and the connection should proceed; `false` if the
        /// certificate chain is invalid and the connection should be aborted.
        /// @throws Ice::SecurityException if the certificate chain is invalid and the connection should be aborted.
        /// @see SSL::OpenSSLConnectionInfo
        /// @see SSL::SecureTransportConnectionInfo
        /// @see SSL::SchannelConnectionInfo
        std::function<bool(CtxtHandle context, const ConnectionInfoPtr& info)> clientCertificateValidationCallback;
    };

    /// @cond INTERNAL
    /// An alias for the platform-specific implementation of ClientAuthenticationOptions on Windows.
    using ServerAuthenticationOptions = SchannelServerAuthenticationOptions;
    /// @endcond
#endif

#if defined(ICE_USE_SECURE_TRANSPORT) || defined(ICE_DOXYGEN)
    /// SSL transport options for server connections on macOS and iOS.
    ///
    /// The SecureTransportServerAuthenticationOptions structure is only available when the Ice library is built on
    /// macOS and iOS. For Linux, refer to OpenSSLServerAuthenticationOptions, and for Windows, refer to
    /// SchannelServerAuthenticationOptions.
    /// @see ::Ice::SSL::ServerAuthenticationOptions
    struct SecureTransportServerAuthenticationOptions
    {
        /// A callback for selecting the server's SSL certificate chain based on the name of the object adapter that
        /// accepts the connection.
        ///
        /// This callback is invoked by the SSL transport for each new incoming connection before starting the SSL
        /// handshake to determine the appropriate server certificate chain. The callback must return a `CFArrayRef`
        /// that represents the server's certificate chain, or nullptr if no certificate chain should be used for the
        /// connection. The SSL transport takes ownership of the returned `CFArrayRef` and releases it when the
        /// connection is closed.
        ///
        /// @param adapterName The name of the object adapter that accepted the connection.
        /// @return A `CFArrayRef` containing the server's certificate chain, or nullptr to indicate that no certificate
        /// is used.
        ///
        /// Example of setting `serverCertificateSelectionCallback`:
        /// @snippet Ice/SSL/SecureTransportServerAuthenticationOptions.cpp serverCertificateSelectionCallback
        ///
        /// See the [SSLSetCertificate] documentation for requirements on the certificate chain format.
        ///
        /// [SSLSetCertificate]:
        /// https://developer.apple.com/documentation/security/1392400-sslsetcertificate?changes=_3&language=objc
        std::function<CFArrayRef(const std::string& adapterName)> serverCertificateSelectionCallback;

        /// A callback invoked before initiating a new SSL handshake, providing an opportunity to customize the SSL
        /// parameters for the session based on specific server settings or requirements.
        ///
        /// @param context An opaque type that represents an SSL session context object.
        /// @param adapterName The name of the object adapter that accepted the connection.
        ///
        /// Example of setting `sslNewSessionCallback`:
        /// @snippet Ice/SSL/SecureTransportServerAuthenticationOptions.cpp sslNewSessionCallback
        std::function<void(SSLContextRef context, const std::string& adapterName)> sslNewSessionCallback;

        /// The requirements for client-side authentication. The default is `kNeverAuthenticate`.
        ///
        /// [see SSLAuthenticate](https://developer.apple.com/documentation/security/sslauthenticate)
        SSLAuthenticate clientCertificateRequired = kNeverAuthenticate;

        /// The trusted root certificates used for validating the client's certificate chain. If this field is set, the
        /// client's certificate chain is validated against these certificates; otherwise, the system's default root
        /// certificates are used.
        ///
        /// The trusted root certificates are used by both the default validation callback and by custom validation
        /// callbacks set in `clientCertificateValidationCallback`.
        ///
        /// This is equivalent to calling [SecTrustSetAnchorCertificates] with the `CFArrayRef` object, and
        /// [SecTrustSetAnchorCertificatesOnly] with the `anchorCertificatesOnly` parameter set to true.
        ///
        /// Example of setting `trustedRootCertificates`:
        /// @snippet Ice/SSL/SecureTransportServerAuthenticationOptions.cpp trustedRootCertificates
        ///
        /// [SecTrustSetAnchorCertificates]:
        /// https://developer.apple.com/documentation/security/1396098-sectrustsetanchorcertificates?language=objc
        /// [SecTrustSetAnchorCertificatesOnly]:
        /// https://developer.apple.com/documentation/security/1399071-sectrustsetanchorcertificatesonl?language=objc
        CFArrayRef trustedRootCertificates = nullptr;

        /// A callback for validating the client certificate chain. If the verification callback returns false, the
        /// connection will be aborted with an Ice::SecurityException.
        ///
        /// The client certificate chain is validated using the trust object passed to the callback. When this callback
        /// is set, it replaces the default validation callback and must perform all necessary validation steps. If
        /// `trustedRootCertificates` is set, the passed trust object will use them as the anchor certificates for
        /// evaluating trust. This setting can be modified by the application using [SecTrustSetAnchorCertificates].
        ///
        /// Example of setting `clientCertificateValidationCallback`:
        /// @snippet Ice/SSL/SecureTransportServerAuthenticationOptions.cpp clientCertificateValidationCallback
        ///
        /// @param trust The trust object that contains the client's certificate chain.
        /// @param info The connection info object that provides additional connection-related data. The
        /// `ConnectionInfo` type is an alias for the platform-specific connection info class.
        /// @return `true` if the certificate chain is valid and the connection should proceed; `false` if the
        /// certificate chain is invalid and the connection should be aborted.
        /// @throws Ice::SecurityException if the certificate chain is invalid and the connection should be aborted.
        ///
        /// @see [SecTrustEvaluateWithError]
        /// @see SSL::OpenSSLConnectionInfo
        /// @see SSL::SecureTransportConnectionInfo
        /// @see SSL::SchannelConnectionInfo
        ///
        /// [SecTrustSetAnchorCertificates]:
        /// https://developer.apple.com/documentation/security/1396098-sectrustsetanchorcertificates?language=objc
        /// [SecTrustEvaluateWithError]:
        /// https://developer.apple.com/documentation/security/2980705-sectrustevaluatewitherror?language=objc
        std::function<bool(SecTrustRef trust, ConnectionInfoPtr info)> clientCertificateValidationCallback;
    };

    /// @cond INTERNAL
    // An alias for the platform-specific implementation of ClientAuthenticationOptions on macOS and iOS.
    using ServerAuthenticationOptions = SecureTransportServerAuthenticationOptions;
    /// @endcond
#endif

#if defined(ICE_USE_OPENSSL) || defined(ICE_DOXYGEN)
    /// SSL transport options for server connections on Linux.
    ///
    /// The OpenSSLServerAuthenticationOptions structure is only available when the Ice library is built on
    /// Linux. For macOS and iOS, refer to SecureTransportServerAuthenticationOptions, and for Windows, refer to
    /// SchannelServerAuthenticationOptions.
    /// @see ::Ice::SSL::ServerAuthenticationOptions
    struct OpenSSLServerAuthenticationOptions
    {
        /// A callback that selects the server's [SSL_CTX][SSL_CTX_new] object based on the name of the object
        /// adapter that accepted the connection.
        ///
        /// This callback associates a specific SSL configuration with an incoming connection identified by
        /// the name of the object adapter that accepted the connection. The callback must return a pointer to a valid
        /// `SSL_CTX` object previously initialized using the OpenSSL API. The SSL transport takes ownership of the
        /// returned `SSL_CTX` object and releases it after closing the connection.
        ///
        /// If the application does not provide a callback, the SSL transport will use an `SSL_CTX` object created with
        /// `SSL_CTX_new()`, which uses the default OpenSSL configuration.
        ///
        /// The SSL transport calls this callback for each new incoming connection to obtain the `SSL_CTX` object
        /// before starting the SSL handshake.
        ///
        /// @param adapterName The name of the object adapter that accepted the connection.
        /// @return A pointer to an `SSL_CTX` object representing the SSL configuration for the new incoming
        /// connection.
        ///
        /// Example of setting `serverSSLContextSelectionCallback`:
        /// @snippet Ice/SSL/OpenSSLServerAuthenticationOptions.cpp serverSSLContextSelectionCallback
        ///
        /// [SSL_CTX_new]: https://www.openssl.org/docs/manmaster/man3/SSL_CTX_new.html
        std::function<SSL_CTX*(const std::string& adapterName)> serverSSLContextSelectionCallback{};

        /// A callback invoked before initiating a new SSL handshake, providing an opportunity to customize the SSL
        /// parameters for the connection.
        ///
        /// @param ssl A pointer to the SSL object representing the connection.
        /// @param adapterName The name of the object adapter that accepted the connection.
        ///
        /// Example of setting `sslNewSessionCallback`:
        /// @snippet Ice/SSL/OpenSSLServerAuthenticationOptions.cpp sslNewSessionCallback
        ///
        /// @see [SSL_new].
        ///
        /// [SSL_new]: https://www.openssl.org/docs/manmaster/man3/SSL_new.html
        std::function<void(::SSL* ssl, const std::string& adapterName)> sslNewSessionCallback{};

        /// A callback for validating the client certificate chain. If the verification callback returns false,
        /// the connection will be aborted with an Ice::SecurityException.
        ///
        /// @param verified A boolean indicating whether the preliminary certificate verification performed by OpenSSL's
        /// built-in mechanisms succeeded or failed. `true` if the preliminary checks passed, `false` otherwise.
        /// @param ctx A pointer to an `X509_STORE_CTX` object, which contains the certificate chain to be verified.
        /// @param info The connection info object that provides additional connection-related data. The
        /// `ConnectionInfo` type is an alias for the platform-specific connection info class.
        /// @return `true` if the certificate chain is valid and the connection should proceed; `false` if the
        /// certificate chain is invalid and the connection should be aborted.
        /// @throws Ice::SecurityException if the certificate chain is invalid and the connection should be aborted.
        ///
        /// Example of setting `clientCertificateValidationCallback`:
        /// @snippet Ice/SSL/OpenSSLServerAuthenticationOptions.cpp clientCertificateValidationCallback
        ///
        /// @see [Certificate verification in OpenSSL][SSL_set_verify].
        ///
        /// [SSL_set_verify]: https://www.openssl.org/docs/manmaster/man3/SSL_set_verify.html
        /// @see SSL::OpenSSLConnectionInfo
        /// @see SSL::SecureTransportConnectionInfo
        /// @see SSL::SchannelConnectionInfo
        std::function<bool(bool verified, X509_STORE_CTX* ctx, const ConnectionInfoPtr& info)>
            clientCertificateValidationCallback{};
    };

    /// @cond INTERNAL
    // An alias for the platform-specific implementation of ClientAuthenticationOptions on Linux.
    using ServerAuthenticationOptions = OpenSSLServerAuthenticationOptions;
    /// @endcond
#endif
}

#endif
