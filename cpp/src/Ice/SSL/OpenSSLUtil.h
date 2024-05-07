//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SSL_OPENSSL_UTIL_I_H
#define ICE_SSL_OPENSSL_UTIL_I_H

#include <openssl/ssl.h>

#include <string>

namespace Ice::SSL::OpenSSL
{
    // Accumulate the OpenSSL error stack into a string.
    std::string getErrors(bool);

}
#endif
