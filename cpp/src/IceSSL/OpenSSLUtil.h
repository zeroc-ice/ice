// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_OPENSSL_UTIL_I_H
#define ICESSL_OPENSSL_UTIL_I_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Handle.h>

#include <openssl/ssl.h>

#include <list>

namespace IceSSL
{

namespace OpenSSL
{

#ifndef OPENSSL_NO_DH

class DHParams : public IceUtil::Shared, public IceUtil::Mutex
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

    DH* _dh512;
    DH* _dh1024;
    DH* _dh2048;
    DH* _dh4096;
};
typedef IceUtil::Handle<DHParams> DHParamsPtr;

#endif

//
// Accumulate the OpenSSL error stack into a string.
//
std::string getSslErrors(bool);

} // OpenSSL namespace end

} // IceSSL namespace end

#endif
