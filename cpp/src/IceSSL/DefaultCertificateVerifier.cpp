// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Logger.h>
#include <IceSSL/OpenSSL.h>
#include <IceSSL/DefaultCertificateVerifier.h>
#include <IceSSL/OpenSSLUtils.h>
#include <IceSSL/TraceLevels.h>

#include <ostream>

using namespace std;

IceSSL::OpenSSL::DefaultCertificateVerifier::DefaultCertificateVerifier(const IceSSL::TraceLevelsPtr& traceLevels,
                                                                        const Ice::LoggerPtr& logger) :
    _traceLevels(traceLevels),
    _logger(logger)
{
}

int
IceSSL::OpenSSL::DefaultCertificateVerifier::verify(int preVerifyOkay, X509_STORE_CTX* x509StoreContext,
                                                    SSL* sslConnection)
{
    //
    // Default verification steps.
    //

    int verifyError = X509_STORE_CTX_get_error(x509StoreContext);
    int errorDepth = X509_STORE_CTX_get_error_depth(x509StoreContext);
    int verifyDepth = SSL_get_verify_depth(sslConnection);

    // A verify error has been encountered.
    if (verifyError != X509_V_OK)
    {
        // We have a limited verify depth, and we have had to delve too deeply
        // into the certificate chain to find an acceptable root certificate.
        if ((verifyDepth != -1) && (verifyDepth < errorDepth))
        {
            verifyError = X509_V_ERR_CERT_CHAIN_TOO_LONG;
            X509_STORE_CTX_set_error(x509StoreContext, verifyError);
        }

        // If we have ANY errors, we bail out.
        preVerifyOkay = 0;
    }

    // Only if ICE_PROTOCOL level logging is on do we worry about this.
    if (_traceLevels->security >= IceSSL::SECURITY_PROTOCOL)
    {
        char buf[256];

        X509* err_cert = X509_STORE_CTX_get_current_cert(x509StoreContext);

        X509_NAME_oneline(X509_get_subject_name(err_cert), buf, sizeof(buf));

        ostringstream outStringStream;

        outStringStream << "depth = " << dec << errorDepth << ":" << buf << std::endl;

        if (!preVerifyOkay)
        {
            outStringStream << "verify error: num = " << verifyError << " : " 
			    << X509_verify_cert_error_string(verifyError) << endl;

        }

        switch (verifyError)
        {
            case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
            {
                X509_NAME_oneline(X509_get_issuer_name(err_cert), buf, sizeof(buf));
                outStringStream << "issuer = " << buf << endl;
                break;
            }

            case X509_V_ERR_CERT_NOT_YET_VALID:
            case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
            {
                outStringStream << "notBefore = " << getASN1time(X509_get_notBefore(err_cert)) << endl;
                break;
            }

            case X509_V_ERR_CERT_HAS_EXPIRED:
            case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
            {
                outStringStream << "notAfter = " << getASN1time(X509_get_notAfter(err_cert)) << endl;
                break;
            }
        }

        outStringStream << "verify return = " << preVerifyOkay << endl;

        _logger->trace(_traceLevels->securityCat, outStringStream.str());
    }

    return preVerifyOkay;
}
