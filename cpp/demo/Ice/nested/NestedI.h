// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef NESTED_I_H
#define NESTED_I_H

#include <Nested.h>

class NestedI : public Nested
{
public:

    NestedI(const NestedPrx&);
    virtual void nestedCall(Ice::Int, const NestedPrx&, const Ice::Current&);

private:

    NestedPrx _self;
};

#endif
