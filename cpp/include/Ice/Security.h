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

namespace IceSSL
{

using std::string;

typedef enum
{
    None = 0,
    Client,
    Server,
    ClientServer
} ICE_API SslContextType;

void ICE_API setSystemCertificateVerifier(SslContextType,
                                          const CertificateVerifierPtr&,
                                          const IceInternal::InstancePtr& instance);

void ICE_API setSystemCertAuthCertificate(SslContextType,
                                          const string&,
                                          const IceInternal::InstancePtr& instance);

void ICE_API setSystemRSAKeysBase64(SslContextType,
                                    const string&,
                                    const string&,
                                    const IceInternal::InstancePtr& instance);

}

#endif

