//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef SLICE_MD5_H
#define SLICE_MD5_H

#include <IceUtil/Config.h>

extern "C"
{
struct md5_state_s;
}

namespace Slice
{

class MD5
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
