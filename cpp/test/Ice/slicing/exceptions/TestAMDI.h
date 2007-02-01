// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TESTAMDI_H
#define TESTAMDI_H

#include <ServerPrivateAMD.h>

class TestI : virtual public Test::TestIntf
{
public:

    TestI(const ::Ice::ObjectAdapterPtr&);
    virtual void baseAsBase_async(const ::Test::AMD_TestIntf_baseAsBasePtr&, const ::Ice::Current&);

    virtual void unknownDerivedAsBase_async(const ::Test::AMD_TestIntf_unknownDerivedAsBasePtr&, const ::Ice::Current&);
    virtual void knownDerivedAsBase_async(const ::Test::AMD_TestIntf_knownDerivedAsBasePtr&, const ::Ice::Current&);
    virtual void knownDerivedAsKnownDerived_async(const ::Test::AMD_TestIntf_knownDerivedAsKnownDerivedPtr&,
                                                  const ::Ice::Current&);

    virtual void unknownIntermediateAsBase_async(const ::Test::AMD_TestIntf_unknownIntermediateAsBasePtr&,
                                                 const ::Ice::Current&);
    virtual void knownIntermediateAsBase_async(const ::Test::AMD_TestIntf_knownIntermediateAsBasePtr&,
                                               const ::Ice::Current&);
    virtual void knownMostDerivedAsBase_async(const ::Test::AMD_TestIntf_knownMostDerivedAsBasePtr&,
                                              const ::Ice::Current&);
    virtual void knownIntermediateAsKnownIntermediate_async(
                    const ::Test::AMD_TestIntf_knownIntermediateAsKnownIntermediatePtr&,
                    const ::Ice::Current&);
    virtual void knownMostDerivedAsKnownIntermediate_async(
                    const ::Test::AMD_TestIntf_knownMostDerivedAsKnownIntermediatePtr&,
                    const ::Ice::Current&);
    virtual void knownMostDerivedAsKnownMostDerived_async(
                    const ::Test::AMD_TestIntf_knownMostDerivedAsKnownMostDerivedPtr&,
                    const ::Ice::Current&);

    virtual void unknownMostDerived1AsBase_async(
                    const ::Test::AMD_TestIntf_unknownMostDerived1AsBasePtr&,
                    const ::Ice::Current&);
    virtual void unknownMostDerived1AsKnownIntermediate_async(
                                            const ::Test::AMD_TestIntf_unknownMostDerived1AsKnownIntermediatePtr&,
                                            const ::Ice::Current&);
    virtual void unknownMostDerived2AsBase_async(
                    const ::Test::AMD_TestIntf_unknownMostDerived2AsBasePtr&,
                    const ::Ice::Current&);

    virtual void shutdown_async(const ::Test::AMD_TestIntf_shutdownPtr&, const ::Ice::Current&);

private:

    const ::Ice::ObjectAdapterPtr& _adapter;
};

#endif
