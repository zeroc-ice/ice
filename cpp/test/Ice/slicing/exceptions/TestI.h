// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef TESTI_H
#define TESTI_H

#include <ServerPrivate.h>

class TestI : public virtual Test::TestIntf
{
public:

    TestI();
    virtual void baseAsBase(const ::Ice::Current&);
    virtual void unknownDerivedAsBase(const ::Ice::Current&);
    virtual void knownDerivedAsBase(const ::Ice::Current&);
    virtual void knownDerivedAsKnownDerived(const ::Ice::Current&);

    virtual void unknownIntermediateAsBase(const ::Ice::Current&);
    virtual void knownIntermediateAsBase(const ::Ice::Current&);
    virtual void knownMostDerivedAsBase(const ::Ice::Current&);
    virtual void knownIntermediateAsKnownIntermediate(const ::Ice::Current&);
    virtual void knownMostDerivedAsKnownIntermediate(const ::Ice::Current&);
    virtual void knownMostDerivedAsKnownMostDerived(const ::Ice::Current&);

    virtual void unknownMostDerived1AsBase(const ::Ice::Current&);
    virtual void unknownMostDerived1AsKnownIntermediate(const ::Ice::Current&);
    virtual void unknownMostDerived2AsBase(const ::Ice::Current&);

    virtual void unknownMostDerived2AsBaseCompact(const ::Ice::Current&);

    virtual void knownPreservedAsBase(const ::Ice::Current&);
    virtual void knownPreservedAsKnownPreserved(const ::Ice::Current&);

    virtual void relayKnownPreservedAsBase(ICE_IN(::Test::RelayPrxPtr), const ::Ice::Current&);
    virtual void relayKnownPreservedAsKnownPreserved(ICE_IN(::Test::RelayPrxPtr), const ::Ice::Current&);

    virtual void unknownPreservedAsBase(const ::Ice::Current&);
    virtual void unknownPreservedAsKnownPreserved(const ::Ice::Current&);

    virtual void relayUnknownPreservedAsBase(ICE_IN(::Test::RelayPrxPtr), const ::Ice::Current&);
    virtual void relayUnknownPreservedAsKnownPreserved(ICE_IN(::Test::RelayPrxPtr), const ::Ice::Current&);

    virtual void shutdown(const ::Ice::Current&);
};

#endif
