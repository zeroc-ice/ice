// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
