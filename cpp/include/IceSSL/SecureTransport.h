// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_SECURE_TRANSPORT_H
#define ICESSL_SECURE_TRANSPORT_H

#include <IceSSL/Plugin.h>
#include <Security/Security.h>

namespace IceSSL
{

namespace SecureTransport
{

class Certificate;
ICE_DEFINE_PTR(CertificatePtr, Certificate);

class ICESSL_API Certificate : public virtual IceSSL::Certificate
{
public:

    //
    // Construct a certificate using a native certificate.
    //
    // The Certificate class assumes ownership of the given native
    // certificate.
    //
    static CertificatePtr create(SecCertificateRef);

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
    // The returned reference is only valid for the lifetime of this
    // object. You can increment the reference count of the returned
    // object with CFRetain.
    //
    virtual SecCertificateRef getCert() const = 0;
};

} // SecureTransport namespace end

} // IceSSL namespace end

#endif
