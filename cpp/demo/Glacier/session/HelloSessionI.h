// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

    virtual void stop(const Ice::Current&);
    virtual void hello(const Ice::Current&);

private:

    ::std::string _userId;
    HelloSessionManagerIPtr _manager;
};

#endif
