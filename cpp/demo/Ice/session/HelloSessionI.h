// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef HELLO_SESSION_I_H
#define HELLO_SESSION_I_H

#include <HelloSession.h>
#include <SessionManagerI.h>

class HelloSessionI : public ::Demo::HelloSession
{
public:

    HelloSessionI(const SessionManagerIPtr&);
    ~HelloSessionI();

    virtual void sayHello(const Ice::Current&) const;
    virtual void destroyed(const Ice::Current&);
    virtual void destroy(const Ice::Current&);
    virtual void refresh(const Ice::Current&);

private:

    const SessionManagerIPtr _manager;
};

#endif
