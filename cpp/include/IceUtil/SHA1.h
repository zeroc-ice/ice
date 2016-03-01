// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_SHA1_H
#define ICE_UTIL_SHA1_H

#include <IceUtil/Config.h>
#include <IceUtil/UniquePtr.h>

#include <vector>

namespace IceUtilInternal
{

ICE_UTIL_API void 
sha1(const unsigned char*, std::size_t, std::vector<unsigned char>&);
    
#ifndef ICE_OS_WINRT
class ICE_UTIL_API SHA1
{
public:
    
    SHA1();
    ~SHA1();
    
    void update(const unsigned char*, std::size_t);
    void finalize(std::vector<unsigned char>&);
    
private:
    
    // noncopyable
    SHA1(const SHA1&);
    SHA1 operator=(const SHA1&);
    
    class Hasher;
    IceUtil::UniquePtr<Hasher> _hasher;
};
#endif

}
#endif
