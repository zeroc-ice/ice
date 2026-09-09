// Copyright (c) ZeroC, Inc.

#include <Ice/Ice.h>

#if defined(ICE_USE_APPLE_SSL)
void
serverCertificateSelectionCallbackExample()
{
    Ice::CommunicatorHolder communicator = Ice::initialize();
    //! [serverCertificateSelectionCallback]
    CFArrayRef serverCertificateChain = {};
    // Load the server certificate chain from the keychain using Security
    // framework APIs.
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
            .sslNewSessionCallback =
                [](sec_protocol_options_t secOptions, const std::string&)
            {
                sec_protocol_options_set_min_tls_protocol_version(
                    secOptions,
                    tls_protocol_version_TLSv13);
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
            .clientCertificateRequired = true,
            .clientCertificateValidationCallback =
                [](SecTrustRef trust, const Ice::SSL::ConnectionInfoPtr&)
            { return SecTrustEvaluateWithError(trust, nullptr); }});
    //! [clientCertificateValidationCallback]
}
#endif
