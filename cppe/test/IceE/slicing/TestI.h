// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TESTI_H
#define TESTI_H

#include <ServerPrivate.h>

class TestI : virtual public Test::TestIntf
{
public:

    TestI(const ::IceE::ObjectAdapterPtr&);
    virtual void baseAsBase(const ::IceE::Current&);
    virtual void unknownDerivedAsBase(const ::IceE::Current&);
    virtual void knownDerivedAsBase(const ::IceE::Current&);
    virtual void knownDerivedAsKnownDerived(const ::IceE::Current&);

    virtual void unknownIntermediateAsBase(const ::IceE::Current&);
    virtual void knownIntermediateAsBase(const ::IceE::Current&);
    virtual void knownMostDerivedAsBase(const ::IceE::Current&);
    virtual void knownIntermediateAsKnownIntermediate(const ::IceE::Current&);
    virtual void knownMostDerivedAsKnownIntermediate(const ::IceE::Current&);
    virtual void knownMostDerivedAsKnownMostDerived(const ::IceE::Current&);

    virtual void unknownMostDerived1AsBase(const ::IceE::Current&);
    virtual void unknownMostDerived1AsKnownIntermediate(const ::IceE::Current&);
    virtual void unknownMostDerived2AsBase(const ::IceE::Current&);

    virtual void shutdown(const ::IceE::Current&);

private:

    const ::IceE::ObjectAdapterPtr& _adapter;
};

#endif
