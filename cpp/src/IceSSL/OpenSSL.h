// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
