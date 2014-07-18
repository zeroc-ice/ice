// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CONTEXT_I_H
#define CONTEXT_I_H

#include <Context.h>

class ContextI : public Demo::Context
{
public:

    virtual void call(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
};

#endif
