// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SslCertificateVerifierOpenSSL.h>

IceSecurity::Ssl::OpenSSL::CertificateVerifier::~CertificateVerifier()
{
}

int
IceSecurity::Ssl::OpenSSL::DefaultCertificateVerifier::verify(int preVerifyOkay,
                                                              X509_STORE_CTX* x509StoreContext,
                                                              SSL* sslConnection)
{
    //
    // Default verification steps.
    //

    int verifyError = X509_STORE_CTX_get_error(x509StoreContext);
    int errorDepth = X509_STORE_CTX_get_error_depth(x509StoreContext);
    int verifyDepth = SSL_get_verify_depth(sslConnection);

    // Verify Depth was set
    if (verifyError != X509_V_OK)
    {
        // If we have no errors so far, and the certificate chain is too long
        if ((verifyDepth != -1) && (verifyDepth < errorDepth))
        {
            verifyError = X509_V_ERR_CERT_CHAIN_TOO_LONG;
        }

        // If we have ANY errors, we bail out.
        preVerifyOkay = 0;
    }

    return preVerifyOkay;
}
