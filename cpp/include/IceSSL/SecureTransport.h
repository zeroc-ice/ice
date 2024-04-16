//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_SECURE_TRANSPORT_H
#define ICESSL_SECURE_TRANSPORT_H

#ifdef __APPLE__

#    include "Certificate.h"
#    include <Security/Security.h>

namespace IceSSL::SecureTransport
{
    class Certificate;
    using CertificatePtr = std::shared_ptr<Certificate>;

    /**
     * This convenience class is a wrapper around a native certificate.
     */
    class ICE_API Certificate : public virtual IceSSL::Certificate
    {
    public:
        /**
         * Constructs a certificate using a native certificate.
         * The Certificate class assumes ownership of the given native
         * certificate.
         * @param cert The certificate cert.
         * @return The new certificate instance.
         */
        static CertificatePtr create(SecCertificateRef cert);

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
         * object. You can increment the reference count of the returned
         * object with CFRetain.
         */
        virtual SecCertificateRef getCert() const = 0;
    };
}

#endif

#endif
