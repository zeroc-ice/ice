//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>

void
clientCertificateSelectionCallbackExample()
{
    //! [clientCertificateSelectionCallback]
    PCCERT_CONTEXT clientCertificateChain{};
    // Load the server certificate chain using Schannel APIs.
    // ...

    auto initData = Ice::InitializationData{
        .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
            .clientCertificateSelectionCallback =
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
    CFRelease(clientCertificateChain);
    //! [clientCertificateSelectionCallback]
}

void
clientSetTrustedRootCertificatesExample()
{
    //! [trustedRootCertificates]
    CFArrayRef rootCerts = {};
    // Populate root certs with X.509 trusted root certificates
    // ...

    auto initData = Ice::InitializationData{
        .clientAuthenticationOptions =
            Ice::SSL::ClientAuthenticationOptions{.trustedRootCertificates = rootCerts}};
    auto communicator = Ice::initialize(initData);
    CFRelease(rootCerts); // It is safe to release the rootCerts now.
    //! [trustedRootCertificates]
}

void
serverCertificateValidationCallbackExample()
{
    //! [serverCertificateValidationCallback]
    auto initData = Ice::InitializationData{
        .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
            .serverCertificateValidationCallback =
                [](SecTrustRef trust, const Ice::SSL::ConnectionInfoPtr& info)
            { return SecTrustEvaluateWithError(trust, nullptr); }}};
    //! [serverCertificateValidationCallback]
}
