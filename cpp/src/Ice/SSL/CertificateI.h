//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SSL_CERTIFICATE_I_H
#define ICE_SSL_CERTIFICATE_I_H

#include "Ice/SSL/Certificate.h"

#include <string>
#include <vector>

namespace Ice::SSL
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
    class ICE_API CertificateI : public virtual Ice::SSL::Certificate
    {
    public:
        virtual bool operator!=(const Ice::SSL::Certificate&) const;

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
