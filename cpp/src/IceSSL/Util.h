// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_UTIL_H
#define ICE_SSL_UTIL_H

#include <IceSSL/UtilF.h>
#include <Ice/Network.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>

#include <IceSSL/Plugin.h>

#include <list>
#include <openssl/ssl.h>

namespace IceSSL
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
#endif

//
// Determine if a file or directory exists, with an optional default directory.
//
bool checkPath(std::string&, const std::string&, bool);

//
// Accumulate the OpenSSL error stack into a string.
//
std::string getSslErrors(bool);

}

#endif
