// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
