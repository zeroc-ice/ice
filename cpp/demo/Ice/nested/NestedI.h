// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
