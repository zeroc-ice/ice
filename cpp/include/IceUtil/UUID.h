// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_UUID_H
#define ICE_UTIL_UUID_H

#include <IceUtil/Config.h>
#include <IceUtil/Exception.h>

namespace IceUtil
{

class ICE_UTIL_API UUIDGenerationException : public Exception
{
public:
    
    UUIDGenerationException(const char*, int);
    virtual const char* ice_name() const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

private:

    static const char* _name;
};

ICE_UTIL_API std::string generateUUID();

}

#endif
