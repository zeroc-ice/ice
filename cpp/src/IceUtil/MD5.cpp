// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/MD5.h>
#include <IceUtil/MD5I.h>

using namespace std;

//
// This class is a C++ wrapper around the C implementation contained in
// MD5I.cpp, obtained from http://sourceforge.net/projects/libmd5-rfc/.
//

IceUtil::MD5::MD5()
{
    _state = new md5_state_s;
    md5_init(_state);
}

IceUtil::MD5::MD5(const unsigned char* data, int n)
{
    _state = new md5_state_s;
    md5_init(_state);
    update(data, n);
    finish();
}

IceUtil::MD5::~MD5()
{
    delete _state;
}

void
IceUtil::MD5::update(const unsigned char* data, int n)
{
    md5_append(_state, data, n);
}

void
IceUtil::MD5::finish()
{
    md5_finish(_state, _digest);
    md5_init(_state);
}

void
IceUtil::MD5::getDigest(unsigned char* digest) const
{
    memcpy(digest, _digest, sizeof(unsigned char) * 16);
}
