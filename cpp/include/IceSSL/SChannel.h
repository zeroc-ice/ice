// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_SCHANNEL_H
#define ICESSL_SCHANNEL_H

#include <IceSSL/Plugin.h>

#include <wincrypt.h>

namespace IceSSL
{

namespace SChannel
{

class Certificate;
ICE_DEFINE_PTR(CertificatePtr, Certificate);

/**
 * This convenience class is a wrapper around a native certificate.
 * \headerfile IceSSL/IceSSL.h
 */
class ICESSL_API Certificate : public virtual IceSSL::Certificate
{
public:

    /**
     * Constructs a certificate using a native certificate.
     * The Certificate class assumes ownership of the given native
     * certificate.
     * @param info The certificate data.
     * @return The new certificate instance.
     */
    static CertificatePtr create(CERT_SIGNED_CONTENT_INFO* info);

    /**
     * Loads the certificate from a file. The certificate must use the
     * PEM encoding format.
     * @param file The certificate file.
     * @return The new certificate instance.
     * @throws CertificateReadException if the file cannot be read.
     */
    static CertificatePtr load(const std::string& file);

    /**
     * Decodes a certificate from a string that uses the PEM encoding format.
     * @param str A string containing the encoded certificate.
     * @return The new certificate instance.
     * @throws CertificateEncodingException if an error occurs.
     */
    static CertificatePtr decode(const std::string& str);

    /**
     * Obtains the native X509 certificate value wrapped by this object.
     * @return A reference to the native certificate.
     * The returned reference is only valid for the lifetime of this
     * object. The returned reference is a pointer to a struct.
     */
    virtual CERT_SIGNED_CONTENT_INFO* getCert() const = 0;
};

} // SChannel namespace end

} // IceSSL namespace end

#endif
