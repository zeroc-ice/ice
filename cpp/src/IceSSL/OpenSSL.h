// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OPENSSL_H
#define ICE_OPENSSL_H

#include <IceUtil/Config.h>
#include <openssl/ssl.h>

namespace IceSSL
{

typedef enum
{
    NO_SECURITY_TRACE = 0,
    SECURITY_WARNINGS,
    SECURITY_PARSE_WARNINGS,
    SECURITY_PROTOCOL,
    SECURITY_PROTOCOL_DEBUG
} SecurityTraceLevel;

enum SslProtocol
{
    SSL_V2 = 1,     // Only speak SSLv2
    SSL_V23,        // Speak SSLv2 and SSLv3
    SSL_V3,         // Only speak SSLv3
    TLS_V1          // Only speak TLSv1
};

}

#endif
