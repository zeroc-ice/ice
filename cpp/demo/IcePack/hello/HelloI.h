// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef HELLO_I_H
#define HELLO_I_H

#include <Hello.h>

#ifndef HELLO_API
#   define HELLO_API ICE_DECLSPEC_EXPORT
#endif

class HELLO_API HelloFactoryI : public HelloFactory, public IceUtil::Mutex
{
public:

    HelloFactoryI();
    
    virtual HelloPrx create(const std::string&, const Ice::Current&);

    virtual HelloPrx find(const std::string&, const Ice::Current&) const;

private:

    std::string _id;

};

class HELLO_API HelloI : public Hello
{
public:

    HelloI(const std::string&);

    virtual void sayHello(const Ice::Current&) const;

    virtual void destroy(const Ice::Current&);

private:

    Ice::CommunicatorPtr _communicator;
};

#endif
