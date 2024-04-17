//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_CERTIFICATE_I_H
#define ICESSL_CERTIFICATE_I_H

#include "Ice/Certificate.h"

#include <string>
#include <vector>

namespace IceSSL
{
    //
    // Map a certificate OID to its alias
    //
    struct ICE_API CertificateOID
    {
        const char* name;
        const char* alias;
    };

    extern const ICE_API CertificateOID certificateOIDS[];
    extern const ICE_API int certificateOIDSSize;

    //
    // Certificate common implementation
    //
    class ICE_API CertificateI : public virtual IceSSL::Certificate
    {
    public:
        virtual bool operator!=(const IceSSL::Certificate&) const;

        virtual std::vector<X509ExtensionPtr> getX509Extensions() const;
        virtual X509ExtensionPtr getX509Extension(const std::string&) const;

        virtual bool checkValidity() const;
        virtual bool checkValidity(const std::chrono::system_clock::time_point& now) const;
        virtual std::string toString() const;

    protected:
        // Implementations that support retrieving X509 extensions must reimplement this method to lazzy initialize
        // the extensions list. The default implementation just throw FeatureNotSupportedException.
        virtual void loadX509Extensions() const;
        mutable std::vector<X509ExtensionPtr> _extensions;
    };

} // IceSSL namespace end

#endif
