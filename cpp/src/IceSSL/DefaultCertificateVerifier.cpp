// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <IceSSL/OpenSSL.h>
#include <IceSSL/DefaultCertificateVerifier.h>
#include <IceSSL/OpenSSLUtils.h>
#include <IceSSL/TraceLevels.h>

#include <ostream>

using namespace std;

IceSSL::DefaultCertificateVerifier::DefaultCertificateVerifier(const IceSSL::TraceLevelsPtr& traceLevels,
                                                               const Ice::CommunicatorPtr& communicator) :
    _traceLevels(traceLevels),
    _communicator(communicator)
{
}

int
IceSSL::DefaultCertificateVerifier::verify(int preVerifyOkay, X509_STORE_CTX* x509StoreContext, SSL* sslConnection)
{
    //
    // Default verification steps.
    //

    int verifyError = X509_STORE_CTX_get_error(x509StoreContext);
    int errorDepth = X509_STORE_CTX_get_error_depth(x509StoreContext);
    int verifyDepth = SSL_get_verify_depth(sslConnection);

    // A verify error has been encountered.
    if(verifyError != X509_V_OK)
    {
        // We have a limited verify depth, and we have had to delve too deeply
        // into the certificate chain to find an acceptable root certificate.
        if((verifyDepth != -1) && (verifyDepth < errorDepth))
        {
            verifyError = X509_V_ERR_CERT_CHAIN_TOO_LONG;
            X509_STORE_CTX_set_error(x509StoreContext, verifyError);
        }

        bool checkIgnoreValid = false;

        switch(verifyError)
        {
            case X509_V_ERR_CERT_NOT_YET_VALID:
            case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
            {
                checkIgnoreValid = true;
                break;
            }

            case X509_V_ERR_CERT_HAS_EXPIRED:
            case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
            {
                checkIgnoreValid = true;
                break;
            }

            default :
            {
                // If we have any other errors, we bail out.
                preVerifyOkay = 0;
                break;
            }
        }

        if(checkIgnoreValid)
        {
            ::Ice::PropertiesPtr properties = _communicator->getProperties();

            switch(_contextType)
            {
                case Client :
                {
                    if(properties->getPropertyAsIntWithDefault("IceSSL.Client.IgnoreValidPeriod", 0) == 0)
                    {
                        // Unless we're told to ignore this result, we bail out.
                        preVerifyOkay = 0;
                    }
                    else
                    {
                        preVerifyOkay = 1;
                    }
                    break;
                }

                case Server :
                {
                    if(properties->getPropertyAsIntWithDefault("IceSSL.Server.IgnoreValidPeriod", 0) == 0)
                    {
                        // Unless we're told to ignore this result, we bail out.
                        preVerifyOkay = 0;
                    }
                    else
                    {
                        preVerifyOkay = 1;
                    }
                    break;
                }

                case ClientServer:
                {
                    if(properties->getPropertyAsIntWithDefault("IceSSL.Client.IgnoreValidPeriod", 0) == 0 && 
                        properties->getPropertyAsIntWithDefault("IceSSL.Server.IgnoreValidPeriod", 0) == 0)
                    {
                        // Unless we're told to ignore this result, we bail out.
                        preVerifyOkay = 0;
                    }
                    else
                    {
                        preVerifyOkay = 1;
                    }
                    break;
                }
            }
        }
    }

    // Only if ICE_PROTOCOL level logging is on do we worry about this.
    if(_traceLevels->security >= IceSSL::SECURITY_PROTOCOL)
    {
        char buf[256];

        X509* err_cert = X509_STORE_CTX_get_current_cert(x509StoreContext);

        X509_NAME_oneline(X509_get_subject_name(err_cert), buf, int(sizeof(buf)));

	Ice::Trace out(_communicator->getLogger(), _traceLevels->securityCat);

        out << "depth = " << dec << errorDepth << ":" << buf << "\n";

        if(!preVerifyOkay)
        {
            out << "verify error: num = " << verifyError << " : " 
                << X509_verify_cert_error_string(verifyError) << "\n";

        }

        switch(verifyError)
        {
            case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
            {
                X509_NAME_oneline(X509_get_issuer_name(err_cert), buf, int(sizeof(buf)));
                out << "issuer = " << buf << "\n";
                break;
            }

            case X509_V_ERR_CERT_NOT_YET_VALID:
            case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
            {
                out << "notBefore = " << getASN1time(X509_get_notBefore(err_cert)) << "\n";
                break;
            }

            case X509_V_ERR_CERT_HAS_EXPIRED:
            case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
            {
                out << "notAfter = " << getASN1time(X509_get_notAfter(err_cert)) << "\n";
                break;
            }
        }

        out << "verify return = " << preVerifyOkay << "\n";
    }

    return preVerifyOkay;
}
