// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SECURITY_H
#define ICE_SECURITY_H

#include <Ice/SslCertificateVerifierF.h>
#include <string>

#ifdef WIN32
#   ifdef ICE_API_EXPORTS
#       define ICE_API __declspec(dllexport)
#   else
#       define ICE_API __declspec(dllimport)
#   endif
#else
#   define ICE_API /**/
#endif

namespace IceSecurity
{

namespace Ssl
{

using std::string;

typedef enum
{
    None = 0,
    Client,
    Server,
    ClientServer
} ICE_API SslContextType;

void ICE_API setSystemCertificateVerifier(const string&, SslContextType, const CertificateVerifierPtr&);

void ICE_API setSystemCertAuthCertificate(const string&, SslContextType, const string&);

void ICE_API setSystemRSAKeysBase64(const string&, SslContextType, const string&, const string&);

}

}

#endif

