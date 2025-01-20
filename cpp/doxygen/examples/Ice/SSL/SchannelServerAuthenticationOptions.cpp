// Copyright (c) ZeroC, Inc.

#include <Ice/Ice.h>

using namespace std;

#if defined(ICE_USE_SCHANNEL)

void
serverCertificateSelectionCallbackExample()
{
    Ice::CommunicatorHolder communicator = Ice::initialize();
    //! [serverCertificateSelectionCallback]
    PCCERT_CONTEXT serverCertificateChain{};
    // Load the server certificate chain using Schannel APIs.
    // ...

    communicator->createObjectAdapterWithEndpoints(
        "Hello",
        "ssl -h 127.0.0.1 -p 10000",
        Ice::SSL::ServerAuthenticationOptions{
            .serverCredentialsSelectionCallback = [serverCertificateChain](const string&)
            {
                CertDuplicateCertificateContext(serverCertificateChain);
                return SCH_CREDENTIALS{
                    .dwVersion = SCH_CREDENTIALS_VERSION,
                    .cCreds = 1,
                    .paCred = const_cast<PCCERT_CONTEXT*>(&serverCertificateChain)};
            }});
    communicator->waitForShutdown();

    // Release the server certificate chain when no longer needed
    CertFreeCertificateContext(serverCertificateChain);
    //! [serverCertificateSelectionCallback]
}

void
serverSetTrustedRootCertificatesExample()
{
    Ice::CommunicatorHolder communicator = Ice::initialize();
    //! [trustedRootCertificates]
    HCERTSTORE rootCerts = {};
    // Populate root certs with X.509 trusted root certificates
    // ...

    communicator->createObjectAdapterWithEndpoints(
        "Hello",
        "ssl -h 127.0.0.1 -p 10000",
        Ice::SSL::ServerAuthenticationOptions{.trustedRootCertificates = rootCerts});
    CertCloseStore(rootCerts, 0); // It is safe to close the rootCerts store now.
    //! [trustedRootCertificates]
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
                [](CtxtHandle, const Ice::SSL::ConnectionInfoPtr&) { return true; }});
    //! [clientCertificateValidationCallback]
}
#endif
