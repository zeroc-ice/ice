// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CERTIFICATE_AUTHORITY_H
#define ICE_SSL_CERTIFICATE_AUTHORITY_H

#include <IceUtil/Config.h>

namespace IceSSL
{

class CertificateAuthority
{
public:

    CertificateAuthority();
    CertificateAuthority(std::string&, std::string&);
    CertificateAuthority(CertificateAuthority&);

    void setCAFileName(std::string&);
    void setCAPath(std::string&);

    const std::string& getCAFileName() const;
    const std::string& getCAPath() const;

private:

    std::string _fileName;
    std::string _path;
};

}

#endif
