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

using namespace std;

class CertificateAuthority
{

public:
    CertificateAuthority();
    CertificateAuthority(string&, string&);
    CertificateAuthority(CertificateAuthority&);

    void setCAFileName(string&);
    void setCAPath(string&);

    inline const string& getCAFileName() const { return _fileName; };
    inline const string& getCAPath() const { return _path; };

private:
    string _fileName;
    string _path;
};

}

}

#endif
