// **********************************************************************
//
// Copyright (c) 2003
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
    SSL_V23 = 1,    // Speak SSLv3 and TLSv1
    SSL_V3,         // Only speak SSLv3
    TLS_V1          // Only speak TLSv1
};

}

#endif
