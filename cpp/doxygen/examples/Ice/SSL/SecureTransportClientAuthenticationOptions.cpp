// Copyright (c) ZeroC, Inc.

#include <Ice/Ice.h>
// Disable deprecation warnings from SecureTransport APIs
#include "../../src/Ice/DisableWarnings.h"

#if defined(ICE_USE_SECURE_TRANSPORT)
// NOLINTBEGIN(clang-analyzer-osx.coreFoundation.CFRetainRelease)

void
clientCertificateSelectionCallbackExample()
{
    //! [clientCertificateSelectionCallback]
    CFArrayRef clientCertificateChain = {};
    // Load the client certificate chain from the keychain using SecureTransport
    // APIs.
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
            .sslNewSessionCallback = [](SSLContextRef context, const std::string&)
            {
                OSStatus status = SSLSetProtocolVersionMin(context, kTLSProtocol13);
                if (status != noErr)
                {
                    // Handle error
                }
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
