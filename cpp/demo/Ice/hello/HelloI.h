// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
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

    virtual void sayHello(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    Ice::CommunicatorPtr _communicator;
};

#endif
