// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_RANDOM_H
#define ICE_UTIL_RANDOM_H

#include <IceUtil/Config.h>
#include <IceUtil/Exception.h>

namespace IceUtil
{

class ICE_UTIL_API RandomGeneratorException : public Exception
{
public:
    
    RandomGeneratorException(const char*, int, int = 0);
    virtual std::string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

    int error() const;

private:

    const int _error;
    static const char* _name;
};

ICE_UTIL_API void generateRandom(char*, int);
ICE_UTIL_API int random(int = 0);

}

#endif
