//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_OPENSSL_UTIL_I_H
#define ICESSL_OPENSSL_UTIL_I_H

#include <openssl/ssl.h>

#include <string>

namespace IceSSL::OpenSSL
{
    // Accumulate the OpenSSL error stack into a string.
    std::string getSslErrors(bool);

}
#endif
