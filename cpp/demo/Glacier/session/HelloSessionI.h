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

#ifndef HELLO_SESSION_I_H
#define HELLO_SESSION_I_H

#include <Glacier/SessionManager.h>
#include <HelloSession.h>

class HelloSessionManagerI : public Glacier::SessionManager
{
public:

    HelloSessionManagerI(const Ice::ObjectAdapterPtr&);

    virtual Glacier::SessionPrx create(const ::std::string&, const Ice::Current&);

    void remove(const Ice::Identity&);

private:

    Ice::ObjectAdapterPtr _adapter;
};
typedef IceUtil::Handle<HelloSessionManagerI> HelloSessionManagerIPtr;

class HelloSessionI : public HelloSession
{
public:

    HelloSessionI(const ::std::string&, const HelloSessionManagerIPtr&);

    virtual void destroy(const Ice::Current&);
    virtual void hello(const Ice::Current&);

private:

    ::std::string _userId;
    HelloSessionManagerIPtr _manager;
};

#endif
