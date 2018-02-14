// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_UWP_H
#define ICESSL_UWP_H

#include <IceSSL/Plugin.h>

namespace IceSSL
{

namespace UWP
{

class Certificate;
ICE_DEFINE_PTR(CertificatePtr, Certificate);

class ICESSL_API Certificate : public virtual IceSSL::Certificate
{
public:

    //
    // Construct a certificate using a native certificate.
    //
    static CertificatePtr create(Windows::Security::Cryptography::Certificates::Certificate^);

    //
    // Load the certificate from a file. The certificate must use the
    // PEM encoding format. Raises CertificateReadException if the
    // file cannot be read.
    //
    static CertificatePtr load(const std::string&);

    //
    // Decode a certificate from a string that uses the PEM encoding
    // format.  Raises CertificateEncodingException if an error
    // occurs.
    //
    static CertificatePtr decode(const std::string&);

    //
    // Retrieve the native X509 certificate value wrapped by this
    // object.
    //
    virtual Windows::Security::Cryptography::Certificates::Certificate^ getCert() const = 0;
};

} // UWP namespace end

} // IceSSL namespace end

#endif
