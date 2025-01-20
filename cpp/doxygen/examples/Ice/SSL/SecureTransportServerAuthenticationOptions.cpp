// Copyright (c) ZeroC, Inc.

#include <Ice/Ice.h>
// Disable deprecation warnings from SecureTransport APIs
#include "../../src/Ice/DisableWarnings.h"

#if defined(ICE_USE_SECURE_TRANSPORT)
void
serverCertificateSelectionCallbackExample()
{
    Ice::CommunicatorHolder communicator = Ice::initialize();
    //! [serverCertificateSelectionCallback]
    CFArrayRef serverCertificateChain = {};
    // Load the server certificate chain from the keychain using SecureTransport
    // APIs.
    communicator->createObjectAdapterWithEndpoints(
        "Hello",
        "ssl -h 127.0.0.1 -p 10000",
        Ice::SSL::ServerAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificateChain](const std::string&)
            {
                // Retain the server certificate chain to ensure it remains
                // valid for the duration of the connection. The SSL transport
                // will release it after closing the connection.
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            }});
    communicator->waitForShutdown();
    // Release the CFArrayRef when no longer needed
    CFRelease(serverCertificateChain);
    //! [serverCertificateSelectionCallback]
}

void
serverSetTrustedRootCertificatesExample()
{
    Ice::CommunicatorHolder communicator = Ice::initialize();
    //! [trustedRootCertificates]
    CFArrayRef rootCerts = {};
    // Populate root certs with X.509 trusted root certificates
    communicator->createObjectAdapterWithEndpoints(
        "Hello",
        "ssl -h 127.0.0.1 -p 10000",
        Ice::SSL::ServerAuthenticationOptions{.trustedRootCertificates = rootCerts});
    CFRelease(rootCerts); // It is safe to release the rootCerts now.
    //! [trustedRootCertificates]
}

void
serverSetNewSessionCallbackExample()
{
    Ice::CommunicatorHolder communicator = Ice::initialize();
    //! [sslNewSessionCallback]
    communicator->createObjectAdapterWithEndpoints(
        "Hello",
        "ssl -h 127.0.0.1 -p 10000",
        Ice::SSL::ServerAuthenticationOptions{
            .sslNewSessionCallback = [](SSLContextRef context, const std::string&)
            {
                OSStatus status = SSLSetProtocolVersionMin(context, kTLSProtocol13);
                if (status != noErr)
                {
                    // Handle error
                }
            }});
    //! [sslNewSessionCallback]
}

void
clientCertificateValidationCallbackExample()
{
    Ice::CommunicatorHolder communicator = Ice::initialize();
    //! [clientCertificateValidationCallback]
    communicator->createObjectAdapterWithEndpoints(
        "Hello",
        "ssl -h 127.0.0.1 -p 10000",
        Ice::SSL::ServerAuthenticationOptions{
            .clientCertificateRequired = kAlwaysAuthenticate,
            .clientCertificateValidationCallback =
                [](SecTrustRef trust, const Ice::SSL::ConnectionInfoPtr&)
            { return SecTrustEvaluateWithError(trust, nullptr); }});
    //! [clientCertificateValidationCallback]
}
#endif
