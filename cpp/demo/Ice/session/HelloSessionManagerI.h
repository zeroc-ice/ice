// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef HELLO_SESSION_MANAGER_I_H
#define HELLO_SESSION_MANAGER_I_H

#include <SessionManagerI.h>

class HelloSessionManagerI : public SessionManagerI
{
public:

    HelloSessionManagerI(const Ice::CommunicatorPtr&);

    virtual ::Demo::SessionPrx create(const ::Ice::Current&);
};

#endif
