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

#if defined(_WIN32)
#   define HELLO_API __declspec(dllexport)
#else
#   define HELLO_API /**/
#endif

class HELLO_API HelloI : public Hello
{
public:

    HelloI(const Ice::CommunicatorPtr&);

    virtual void sayHello(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    Ice::CommunicatorPtr _communicator;
};

#endif
