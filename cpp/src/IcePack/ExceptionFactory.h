// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_EXCEPTION_FACTORY_H
#define ICE_PACK_EXCEPTION_FACTORY_H

#include <Ice/UserExceptionFactory.h>

namespace IcePack
{

class ExceptionFactory : public Ice::UserExceptionFactory
{
public:
    
    ExceptionFactory(const Ice::CommunicatorPtr&);

    virtual void createAndThrow(const std::string&);
    virtual void destroy();
};

}

#endif
