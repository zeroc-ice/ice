// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_UTIL_H
#define ICE_SSL_UTIL_H

#include <UtilF.h>
#include <Ice/Network.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>

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
// Wait for a socket to become readable.
//
bool selectRead(SOCKET, int);

//
// Wait for a socket to become writeable.
//
bool selectWrite(SOCKET, int);

bool splitString(const std::string&, const std::string&, bool, std::vector<std::string>&);

}

#endif
