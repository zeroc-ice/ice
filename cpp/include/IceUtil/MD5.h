// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_MD5_H
#define ICE_UTIL_MD5_H

#include <IceUtil/Config.h>

extern "C"
{
struct md5_state_s;
}

namespace IceUtil
{

class ICE_UTIL_API MD5
{
public:

    MD5();
    MD5(const unsigned char*, int);
    ~MD5();

    void update(const unsigned char*, int);
    void finish();

    void getDigest(unsigned char*) const;

private:

    md5_state_s* _state;
    unsigned char _digest[16];
};

}

#endif
