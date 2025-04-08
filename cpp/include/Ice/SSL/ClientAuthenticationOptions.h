// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_CLIENT_AUTHENTICATION_OPTIONS_H
#define ICE_SSL_CLIENT_AUTHENTICATION_OPTIONS_H

#include "Config.h"
#include "ConnectionInfo.h"

#include <functional>

// This file defines the `XxxClientAuthenticationOptions` structure for each platform-specific SSL implementation. The
// `#if defined(ICE_USE_XXX)/#endif` directives are used to include the appropriate structure based on the platform. We
// avoid using `#elif` directives because we want to define all the structures when building the doxygen documentation.

namespace Ice::SSL
{
#if defined(ICE_DOXYGEN)
    /// An alias for the platform-specific implementation of the SSL %ClientAuthenticationOptions.
    using ClientAuthenticationOptions = ...;
#endif

#if defined(ICE_USE_SCHANNEL) || defined(ICE_DOXYGEN)
    /// SSL transport options for client connections on Windows.
    ///
    /// The SchannelClientAuthenticationOptions structure is only available when the Ice library is built on Windows.
    /// For Linux, refer to OpenSSLClientAuthenticationOptions, and for macOS and iOS, refer to
    /// SecureTransportClientAuthenticationOptions.
    /// @see ::Ice::SSL::ClientAuthenticationOptions
    struct SchannelClientAuthenticationOptions
    {
        /// A callback for selecting the client's SSL credentials based on the target host name.
        ///
        /// This callback is invoked by the SSL transport for each new outgoing connection before starting the SSL
        /// handshake to determine the appropriate client credentials. The callback must return a `SCH_CREDENTIALS` that
        /// represents the client's credentials. The SSL transport takes ownership of the credentials' `paCred` and
        /// `hRootStore` members and releases them when the connection is closed.
        ///
        /// @param host The target host name.
        /// @return The client SSL credentials.
        ///
        /// Example of setting `clientCertificateSelectionCallback`:
        /// @snippet Ice/SSL/SchannelClientAuthenticationOptions.cpp clientCertificateSelectionCallback
        ///
        /// @see [SCH_CREDENTIALS]
        ///
        /// [SCH_CREDENTIALS]:
        /// https://learn.microsoft.com/en-us/windows/win32/api/schannel/ns-schannel-sch_credentials
        std::function<SCH_CREDENTIALS(const std::string& host)> clientCredentialsSelectionCallback;

        /// A callback invoked before initiating a new SSL handshake, providing an opportunity to customize the SSL
        /// parameters for the session based on specific client settings or requirements.
        ///
        /// @param context An opaque type that represents the security context associated with the current connection.
        /// @param host The target host name.
        std::function<void(CtxtHandle context, const std::string& host)> sslNewSessionCallback;

        /// The trusted root certificates used for validating the server's certificate chain. If this field is set, the
        /// server's certificate chain is validated against these certificates; otherwise, the system's default root
        /// certificates are used.
        ///
        /// Example of setting `trustedRootCertificates`:
        /// @snippet Ice/SSL/SchannelClientAuthenticationOptions.cpp trustedRootCertificates
        HCERTSTORE trustedRootCertificates = nullptr;

        /// A callback for validating the server certificate chain. If the verification callback returns false, the
        /// connection will be aborted with an Ice::SecurityException.
        ///
        /// When this callback is set, it replaces the default validation callback and must perform all necessary
        /// validation steps.
        ///
        /// Example of setting `serverCertificateValidationCallback`:
        /// @snippet Ice/SSL/SchannelClientAuthenticationOptions.cpp serverCertificateValidationCallback
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
        std::function<bool(CtxtHandle context, const ConnectionInfoPtr& info)> serverCertificateValidationCallback;
    };

    /// @cond INTERNAL
    /// An alias for the platform-specific implementation of the SSL ClientAuthenticationOptions on Windows.
    using ClientAuthenticationOptions = SchannelClientAuthenticationOptions;
    /// @endcond
#endif

#if defined(ICE_USE_SECURE_TRANSPORT) || defined(ICE_DOXYGEN)
    /// SSL transport options for client connections on macOS and iOS.
    ///
    /// The SecureTransportClientAuthenticationOptions structure is only available when the Ice library is built on
    /// macOS and iOS. For Linux, refer to OpenSSLClientAuthenticationOptions, and for Windows, refer to
    /// SchannelClientAuthenticationOptions.
    /// @see ::Ice::SSL::ClientAuthenticationOptions
    struct SecureTransportClientAuthenticationOptions
    {
        /// A callback for selecting the client's SSL certificate chain based on the target host name.
        ///
        /// This callback is invoked by the SSL transport for each new outgoing connection before starting the SSL
        /// handshake to determine the appropriate client certificate chain. The callback must return a `CFArrayRef`
        /// that represents the client's certificate chain, or nullptr if no certificate chain should be used for the
        /// connection. The SSL transport takes ownership of the returned `CFArrayRef` and releases it when the
        /// connection is closed.
        ///
        /// @param host The target host name.
        /// @return A `CFArrayRef` containing the client's certificate chain, or nullptr to indicate that no certificate
        /// is used.
        ///
        /// Example of setting `clientCertificateSelectionCallback`:
        /// @snippet Ice/SSL/SecureTransportClientAuthenticationOptions.cpp clientCertificateSelectionCallback
        ///
        /// See the [SSLSetCertificate] documentation for requirements on the certificate chain format.
        ///
        /// [SSLSetCertificate]:
        /// https://developer.apple.com/documentation/security/1392400-sslsetcertificate?changes=_3&language=objc
        std::function<CFArrayRef(const std::string& host)> clientCertificateSelectionCallback;

        /// A callback invoked before initiating a new SSL handshake, providing an opportunity to customize the SSL
        /// parameters for the session based on specific client settings or requirements.
        ///
        /// @param context An opaque type that represents an SSL session context object.
        /// @param host The target host name.
        ///
        /// Example of setting `sslNewSessionCallback`:
        /// @snippet Ice/SSL/SecureTransportClientAuthenticationOptions.cpp sslNewSessionCallback
        std::function<void(SSLContextRef context, const std::string& host)> sslNewSessionCallback;

        /// The trusted root certificates used for validating the server's certificate chain. If this field is set, the
        /// server's certificate chain is validated against these certificates; otherwise, the system's default root
        /// certificates are used.
        ///
        /// The trusted root certificates are used by both the default validation callback and by custom validation
        /// callbacks set in `serverCertificateValidationCallback`.
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

        /// A callback for validating the server certificate chain. If the verification callback returns false, the
        /// connection will be aborted with an Ice::SecurityException.
        ///
        /// The server certificate chain is validated using the trust object passed to the callback. When this callback
        /// is set, it replaces the default validation callback and must perform all necessary validation steps. If
        /// `trustedRootCertificates` is set, the passed trust object will use them as the anchor certificates for
        /// evaluating trust. This setting can be modified by the application using [SecTrustSetAnchorCertificates].
        ///
        /// Example of setting `serverCertificateValidationCallback`:
        /// @snippet Ice/SSL/SecureTransportClientAuthenticationOptions.cpp serverCertificateValidationCallback
        ///
        /// @param trust The trust object that contains the server's certificate chain.
        /// @param info The connection info object that provides additional connection-related data. The
        /// `ConnectionInfo` type is an alias for the platform-specific connection info class.
        /// @return `true` if the certificate chain is valid and the connection should proceed; `false` if the
        /// certificate chain is invalid and the connection should be aborted.
        /// @throws Ice::SecurityException if the certificate chain is invalid and the connection should be aborted.
        ///
        /// @see [SecTrustEvaluateWithError]
        ///
        /// [SecTrustSetAnchorCertificates]:
        /// https://developer.apple.com/documentation/security/1396098-sectrustsetanchorcertificates?language=objc
        /// [SecTrustEvaluateWithError]:
        /// https://developer.apple.com/documentation/security/2980705-sectrustevaluatewitherror?language=objc
        /// @see SSL::OpenSSLConnectionInfo
        /// @see SSL::SecureTransportConnectionInfo
        /// @see SSL::SchannelConnectionInfo
        std::function<bool(SecTrustRef trust, const ConnectionInfoPtr& info)> serverCertificateValidationCallback;
    };

    /// @cond INTERNAL
    // An alias for the platform-specific implementation of the SSL ClientAuthenticationOptions on macOS and iOS.
    using ClientAuthenticationOptions = SecureTransportClientAuthenticationOptions;
    /// @endcond
#endif

#if defined(ICE_USE_OPENSSL) || defined(ICE_DOXYGEN)
    /// SSL transport options for client connections on Linux.
    ///
    /// The OpenSSLClientAuthenticationOptions structure is only available when the Ice library is built on
    /// Linux. For macOS and iOS, refer to SecureTransportClientAuthenticationOptions, and for Windows, refer to
    /// SchannelClientAuthenticationOptions.
    /// @see ::Ice::SSL::ClientAuthenticationOptions
    struct OpenSSLClientAuthenticationOptions
    {
        /// A callback that selects the client's [SSL_CTX][SSL_CTX_new] object based on the target host name.
        ///
        /// This callback associates a specific SSL configuration with an outgoing connection identified by the target
        /// host name. The callback must return a pointer to a valid `SSL_CTX` object previously initialized using the
        /// OpenSSL API. The SSL transport takes ownership of the returned `SSL_CTX` object and releases it after
        /// closing the connection.
        ///
        /// If the application does not provide a callback, the SSL transport will use an `SSL_CTX` object created with
        /// `SSL_CTX_new()`, which uses the default OpenSSL configuration.
        ///
        /// The SSL transport calls this callback for each new outgoing connection to obtain the `SSL_CTX` object
        /// before starting the SSL handshake.
        ///
        /// @param host The target host name.
        /// @return A pointer to an `SSL_CTX` object representing the SSL configuration for the new outgoing
        /// connection.
        ///
        /// Example of setting `clientSSLContextSelectionCallback`:
        /// @snippet Ice/SSL/OpenSSLClientAuthenticationOptions.cpp clientSSLContextSelectionCallback
        ///
        /// [SSL_CTX_new]: https://www.openssl.org/docs/manmaster/man3/SSL_CTX_new.html
        std::function<SSL_CTX*(const std::string& host)> clientSSLContextSelectionCallback{};

        /// A callback invoked before initiating a new SSL handshake, providing an opportunity to customize the SSL
        /// parameters for the connection.
        ///
        /// @param ssl A pointer to the SSL object representing the connection.
        /// @param host The target host name.
        ///
        /// Example of setting `sslNewSessionCallback`:
        /// @snippet Ice/SSL/OpenSSLClientAuthenticationOptions.cpp sslNewSessionCallback
        ///
        /// @see [SSL_new].
        ///
        /// [SSL_new]: https://www.openssl.org/docs/manmaster/man3/SSL_new.html
        std::function<void(::SSL* ssl, const std::string& host)> sslNewSessionCallback{};

        /// A callback for validating the server certificate chain. If the verification callback returns false,
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
        /// Example of setting `serverCertificateValidationCallback`:
        /// @snippet Ice/SSL/OpenSSLClientAuthenticationOptions.cpp serverCertificateValidationCallback
        ///
        /// @see [Certificate verification in OpenSSL][SSL_set_verify].
        ///
        /// [SSL_set_verify]: https://www.openssl.org/docs/manmaster/man3/SSL_set_verify.html
        /// @see SSL::OpenSSLConnectionInfo
        /// @see SSL::SecureTransportConnectionInfo
        /// @see SSL::SchannelConnectionInfo
        std::function<bool(bool verified, X509_STORE_CTX* ctx, const ConnectionInfoPtr& info)>
            serverCertificateValidationCallback{};
    };

    /// @cond INTERNAL
    // An alias for the platform-specific implementation of the SSL ClientAuthenticationOptions on Linux.
    using ClientAuthenticationOptions = OpenSSLClientAuthenticationOptions;
    /// @endcond
#endif

}

#endif
