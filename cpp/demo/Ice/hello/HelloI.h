// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef HELLO_I_H
#define HELLO_I_H

#include <Hello.h>

class HelloI : public Hello
{
public:

    HelloI(const Ice::CommunicatorPtr&);

    virtual void hello();
    virtual void shutdown();

private:

    Ice::CommunicatorPtr _communicator;
};

#endif
