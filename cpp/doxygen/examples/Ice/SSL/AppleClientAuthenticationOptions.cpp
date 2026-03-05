// Copyright (c) ZeroC, Inc.

#include <Ice/Ice.h>

#if defined(ICE_USE_APPLE_SSL)
// NOLINTBEGIN(clang-analyzer-osx.coreFoundation.CFRetainRelease)

void
clientCertificateSelectionCallbackExample()
{
    //! [clientCertificateSelectionCallback]
    CFArrayRef clientCertificateChain = {};
    // Load the client certificate chain from the keychain using Security
    // framework APIs.
    auto initData = Ice::InitializationData{
        .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
            .clientCertificateSelectionCallback =
                [clientCertificateChain](const std::string&)
            {
                // Retain the client certificate chain to ensure it remains
                // valid for the duration of the connection. The SSL transport
                // will release it after closing the connection.
                CFRetain(clientCertificateChain);
                return clientCertificateChain;
            }}};
    auto communicator = Ice::initialize(initData);
    // ...
    CFRelease(clientCertificateChain); // Release the CFArrayRef when no longer needed
    //! [clientCertificateSelectionCallback]
}

void
clientSetTrustedRootCertificatesExample()
{
    //! [trustedRootCertificates]
    CFArrayRef rootCerts = {};
    // Populate root certs with X.509 trusted root certificates
    auto initData = Ice::InitializationData{
        .clientAuthenticationOptions =
            Ice::SSL::ClientAuthenticationOptions{.trustedRootCertificates = rootCerts}};
    auto communicator = Ice::initialize(initData);
    CFRelease(rootCerts); // It is safe to release the rootCerts now.
    //! [trustedRootCertificates]
}

void
clientSetNewSessionCallbackExample()
{
    //! [sslNewSessionCallback]
    auto initData = Ice::InitializationData{
        .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
            .sslNewSessionCallback = [](sec_protocol_options_t secOptions, const std::string&)
            {
                sec_protocol_options_set_min_tls_protocol_version(secOptions, tls_protocol_version_TLSv13);
            }}};
    //! [sslNewSessionCallback]
}

void
serverCertificateValidationCallbackExample()
{
    //! [serverCertificateValidationCallback]
    auto initData = Ice::InitializationData{
        .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
            .serverCertificateValidationCallback =
                [](SecTrustRef trust, const Ice::SSL::ConnectionInfoPtr&)
            { return SecTrustEvaluateWithError(trust, nullptr); }}};
    //! [serverCertificateValidationCallback]
}

// NOLINTEND(clang-analyzer-osx.coreFoundation.CFRetainRelease)
#endif
