// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef NESTED_I_H
#define NESTED_I_H

#include <Nested.h>

class NestedI : public Demo::Nested
{
public:

    NestedI(const Demo::NestedPrx&);
    virtual void nestedCall(Ice::Int, const Demo::NestedPrx&, const Ice::Current&);

private:

    const Demo::NestedPrx _self;
};

#endif
