//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_OPENSSL_UTIL_I_H
#define ICESSL_OPENSSL_UTIL_I_H

#include <openssl/ssl.h>

#include <list>
#include <string>
#include <mutex>
#include <memory>

namespace IceSSL
{

namespace OpenSSL
{

#ifndef OPENSSL_NO_DH

class DHParams
{
public:

    DHParams();
    ~DHParams();

    bool add(int, const std::string&);
    DH* get(int);

private:

    typedef std::pair<int, DH*> KeyParamPair;
    typedef std::list<KeyParamPair> ParamList;
    ParamList _params;
    std::mutex _mutex;

    DH* _dh512;
    DH* _dh1024;
    DH* _dh2048;
    DH* _dh4096;
};
using DHParamsPtr = std::shared_ptr<DHParams>;

#endif

//
// Accumulate the OpenSSL error stack into a string.
//
std::string getSslErrors(bool);

} // OpenSSL namespace end

} // IceSSL namespace end

#endif
