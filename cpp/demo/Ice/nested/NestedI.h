// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
