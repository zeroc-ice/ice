// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef HELLO_I_H
#define HELLO_I_H

#include <Hello.h>

#ifndef HELLO_API
#   define HELLO_API ICE_DECLSPEC_EXPORT
#endif

class HELLO_API HelloI : public Demo::Hello
{
public:

    virtual void sayHello(const Ice::Current&);
};

#endif
