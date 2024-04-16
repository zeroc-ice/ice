//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_OPENSSL_H
#define ICESSL_OPENSSL_H

#include "Certificate.h"

#include <openssl/pem.h>
#include <openssl/x509v3.h>

namespace IceSSL::OpenSSL
{
    class Certificate;
    using CertificatePtr = std::shared_ptr<Certificate>;

    /**
     * Encapsulates an OpenSSL X.509 certificate.
     */
    class ICE_API Certificate : public virtual IceSSL::Certificate
    {
    public:
        /**
         * Construct a certificate using a native certificate.
         * The Certificate class assumes ownership of the given native
         * certificate.
         * @param cert The native certificate.
         * @return A new certificate object.
         */
        static CertificatePtr create(x509_st* cert);

        /**
         * Load the certificate from a file. The certificate must use the
         * PEM encoding format.
         * @param file The certificate file.
         * @return A new certificate object.
         * @throws CertificateReadException if the file cannot be read.
         */
        static CertificatePtr load(const std::string& file);

        /**
         * Decode a certificate from a string that uses the PEM encoding format.
         * @param cert A string containing the PEM-encoded certificate.
         * @return A new certificate object.
         * @throws CertificateEncodingException if an error occurs.
         */
        static CertificatePtr decode(const std::string& cert);

        /**
         * Retrieve the native X509 certificate value wrapped by this object.
         * @return The native certificate. The returned reference is only valid for the lifetime of this
         * object. You can increment it with X509_dup.
         */
        virtual x509_st* getCert() const = 0;
    };
}
#endif
