// Copyright (c) ZeroC, Inc.

#include <Ice/Ice.h>

#if defined(ICE_USE_SCHANNEL)
void
clientCertificateSelectionCallbackExample()
{
    //! [clientCertificateSelectionCallback]
    PCCERT_CONTEXT clientCertificateChain{};
    // Load the server certificate chain using Schannel APIs.
    // ...

    auto initData = Ice::InitializationData{
        .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
            .clientCredentialsSelectionCallback =
                [clientCertificateChain](const std::string&)
            {
                CertDuplicateCertificateContext(clientCertificateChain);
                return SCH_CREDENTIALS{
                    .dwVersion = SCH_CREDENTIALS_VERSION,
                    .cCreds = 1,
                    .paCred = const_cast<PCCERT_CONTEXT*>(&clientCertificateChain)};
            }}};
    auto communicator = Ice::initialize(initData);

    // Release the client certificate chain when no longer needed
    CertFreeCertificateContext(clientCertificateChain);
    //! [clientCertificateSelectionCallback]
}

void
clientSetTrustedRootCertificatesExample()
{
    //! [trustedRootCertificates]
    HCERTSTORE rootCerts = {};
    // Populate root certs with X.509 trusted root certificates
    // ...

    auto initData = Ice::InitializationData{
        .clientAuthenticationOptions =
            Ice::SSL::ClientAuthenticationOptions{.trustedRootCertificates = rootCerts}};
    auto communicator = Ice::initialize(initData);
    CertCloseStore(rootCerts, 0); // It is safe to close the rootCerts store now.
    //! [trustedRootCertificates]
}

void
serverCertificateValidationCallbackExample()
{
    //! [serverCertificateValidationCallback]
    auto initData = Ice::InitializationData{
        .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
            .serverCertificateValidationCallback =
                [](CtxtHandle, const Ice::SSL::ConnectionInfoPtr&) { return true; }}};
    //! [serverCertificateValidationCallback]
}
#endif
