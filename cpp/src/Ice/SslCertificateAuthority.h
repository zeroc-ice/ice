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

#include <string>

namespace IceSecurity
{

namespace Ssl
{

class CertificateAuthority
{

public:
    CertificateAuthority();
    CertificateAuthority(std::string&, std::string&);
    CertificateAuthority(CertificateAuthority&);

    void setCAFileName(std::string&);
    void setCAPath(std::string&);

    inline const std::string& getCAFileName() const { return _fileName; };
    inline const std::string& getCAPath() const { return _path; };

private:
    std::string _fileName;
    std::string _path;
};

}

}

#endif
