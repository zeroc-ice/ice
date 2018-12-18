// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_CERTIFICATE_I_H
#define ICESSL_CERTIFICATE_I_H

#include <IceSSL/Plugin.h>

#include <vector>
#include <string>

namespace IceSSL
{

//
// Map a certificate OID to its alias
//
struct ICESSL_API CertificateOID
{
    const char* name;
    const char* alias;
};

extern const ICESSL_API CertificateOID certificateOIDS[];
extern const ICESSL_API int certificateOIDSSize;

//
// Certificate common implementation
//
class ICESSL_API CertificateI : public virtual IceSSL::Certificate
{
public:

    virtual bool operator!=(const IceSSL::Certificate&) const;

    virtual std::vector<X509ExtensionPtr> getX509Extensions() const;
    virtual X509ExtensionPtr getX509Extension(const std::string&) const;

    virtual bool checkValidity() const;
#  ifdef ICE_CPP11_MAPPING
    virtual bool checkValidity(const std::chrono::system_clock::time_point& now) const;
#  else
    virtual bool checkValidity(const IceUtil::Time& now) const;
#  endif

    virtual std::string toString() const;

protected:

    //
    // Implementations that support retrieving X509 extensions must
    // reimplement this method to lazzy initialize the extensions
    // list.
    //
    // The default implementation just throw FeatureNotSupportedException
    //
    virtual void loadX509Extensions() const;
    mutable std::vector<X509ExtensionPtr> _extensions;
};

} // IceSSL namespace end

#endif
