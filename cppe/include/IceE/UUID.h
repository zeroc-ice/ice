// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_UUID_H
#define ICEE_UUID_H

#include <IceE/Exception.h>

namespace IceUtil
{

class ICEE_API UUIDGenerationException : public Exception
{
public:
    
    UUIDGenerationException(const char*, int);
    virtual const std::string ice_name() const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

private:

    static const char* _name;
};

ICEE_API std::string generateUUID();

}

#endif
