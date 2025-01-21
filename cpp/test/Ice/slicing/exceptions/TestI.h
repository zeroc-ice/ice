// Copyright (c) ZeroC, Inc.

#ifndef TESTI_H
#define TESTI_H

#include "ServerPrivate.h"

class TestI final : public Test::TestIntf
{
public:
    TestI();

    void baseAsBase(const Ice::Current&) final;
    void unknownDerivedAsBase(const Ice::Current&) final;
    void knownDerivedAsBase(const Ice::Current&) final;
    void knownDerivedAsKnownDerived(const Ice::Current&) final;

    void unknownIntermediateAsBase(const Ice::Current&) final;
    void knownIntermediateAsBase(const Ice::Current&) final;
    void knownMostDerivedAsBase(const Ice::Current&) final;
    void knownIntermediateAsKnownIntermediate(const Ice::Current&) final;
    void knownMostDerivedAsKnownIntermediate(const Ice::Current&) final;
    void knownMostDerivedAsKnownMostDerived(const Ice::Current&) final;

    void unknownMostDerived1AsBase(const Ice::Current&) final;
    void unknownMostDerived1AsKnownIntermediate(const Ice::Current&) final;
    void unknownMostDerived2AsBase(const Ice::Current&) final;

    void shutdown(const Ice::Current&) final;
};

#endif
