// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SingleCertificateVerifier.h>
#include <Ice/SslIceUtils.h>
#include <openssl/err.h>
#include <algorithm>
#include <iostream>

using namespace std;
using Ice::ByteSeq;

IceSSL::OpenSSL::SingleCertificateVerifier::SingleCertificateVerifier(const ByteSeq& publicKey) :
                                           _publicKey(publicKey)
{
}

int
IceSSL::OpenSSL::SingleCertificateVerifier::verify(int preVerifyOkay,
                                                   X509_STORE_CTX* x509StoreContext,
                                                   SSL* sslConnection)
{
    // Short circuit - if the peer cert wasn't good enough for OpenSSL,
    // it's not good enough for us to bother checking.
    if (preVerifyOkay == 0)
    {
        return preVerifyOkay;
    }

    // For getting the CA certificate
    X509* trustedCert = 0;
    X509_OBJECT trustedObject;

    // Get the peer certificate offered by whoever we're talking to.
    X509* peerCertificate = x509StoreContext->cert;

    // We only bother to do the rest of this if we have something to verify.
    if (peerCertificate)
    {
        // Get the subject name (Not a memory leak, this is how this is used).
        X509_NAME* peerCertName = X509_get_subject_name(peerCertificate);

        // The Trusted Certificate by the same name.
        int retCode = X509_STORE_get_by_subject(x509StoreContext,
                                                X509_LU_X509,
                                                peerCertName,
                                                &trustedObject);

        switch (retCode)
        {
            case X509_LU_X509:
            {
                trustedCert = trustedObject.data.x509;
                break;
            }
    
            case X509_LU_RETRY:
            {
                // Log the error properly.
                X509err(X509_F_X509_VERIFY_CERT, X509_R_SHOULD_RETRY);

                // Drop through intended.
            }
    
            default :
            {
                // Regardless of error, if we can't look up the trusted
                // certificate, then we fail out.

                preVerifyOkay = 0;
                break;
            }
        }
    }

    // Compare, only if we have both.
    if (trustedCert)
    {
        ByteSeq peerByteSeq = toByteSeq(peerCertificate);
        ByteSeq trustedByteSeq = toByteSeq(trustedCert);

        // The presented certificate must exactly match one that is in
        // the certificate store, and that must be the expected certificate.

        preVerifyOkay = (peerByteSeq == trustedByteSeq) &&
                        (_publicKey == peerByteSeq);

        X509_OBJECT_free_contents(&trustedObject);
    }

    return preVerifyOkay;
}

ByteSeq
IceSSL::OpenSSL::SingleCertificateVerifier::toByteSeq(X509* certificate)
{
    ByteSeq certByteSeq;

    // Convert the X509 to a unsigned char buffer.
    unsigned int certSize = i2d_X509(certificate, 0);
    unsigned char* certBuffer = new unsigned char[certSize];
    unsigned char* certPtr = certBuffer;
    i2d_X509(certificate, &certPtr);

    // Yet another conversion to a ByteSeq (easy comparison this way).
    IceSSL::ucharToByteSeq(certBuffer, certSize, certByteSeq);
    delete []certBuffer;

    return certByteSeq;
}

