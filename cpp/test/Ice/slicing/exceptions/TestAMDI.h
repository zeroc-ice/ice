// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TESTAMDI_H
#define TESTAMDI_H

#include <ServerPrivateAMD.h>

class TestI : public virtual Test::TestIntf
{
public:

    TestI();
#ifdef ICE_CPP11_MAPPING
    virtual void baseAsBaseAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void unknownDerivedAsBaseAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void knownDerivedAsBaseAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void knownDerivedAsKnownDerivedAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void unknownIntermediateAsBaseAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void knownIntermediateAsBaseAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void knownMostDerivedAsBaseAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void knownIntermediateAsKnownIntermediateAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void knownMostDerivedAsKnownIntermediateAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void knownMostDerivedAsKnownMostDerivedAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void unknownMostDerived1AsBaseAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void unknownMostDerived1AsKnownIntermediateAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void unknownMostDerived2AsBaseAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void unknownMostDerived2AsBaseCompactAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void knownPreservedAsBaseAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void knownPreservedAsKnownPreservedAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void relayKnownPreservedAsBaseAsync(
        std::shared_ptr<::Test::RelayPrx>,
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void relayKnownPreservedAsKnownPreservedAsync(
        std::shared_ptr<::Test::RelayPrx>,
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void unknownPreservedAsBaseAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void unknownPreservedAsKnownPreservedAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void relayUnknownPreservedAsBaseAsync(
        std::shared_ptr<::Test::RelayPrx>,
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void relayUnknownPreservedAsKnownPreservedAsync(
        std::shared_ptr<::Test::RelayPrx>,
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void shutdownAsync(
        std::function<void ()>,
        std::function<void (std::exception_ptr)>,
        const ::Ice::Current&);
#else
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

    virtual void unknownMostDerived2AsBaseCompact_async(
                    const ::Test::AMD_TestIntf_unknownMostDerived2AsBaseCompactPtr&,
                    const ::Ice::Current&);

    virtual void knownPreservedAsBase_async(
                    const ::Test::AMD_TestIntf_knownPreservedAsBasePtr&,
                    const ::Ice::Current&);

    virtual void knownPreservedAsKnownPreserved_async(
                    const ::Test::AMD_TestIntf_knownPreservedAsKnownPreservedPtr&,
                    const ::Ice::Current&);

    virtual void relayKnownPreservedAsBase_async(
                    const ::Test::AMD_TestIntf_relayKnownPreservedAsBasePtr&,
                    const ::Test::RelayPrx&,
                    const ::Ice::Current&);

    virtual void relayKnownPreservedAsKnownPreserved_async(
                    const ::Test::AMD_TestIntf_relayKnownPreservedAsKnownPreservedPtr&,
                    const ::Test::RelayPrx&,
                    const ::Ice::Current&);

    virtual void unknownPreservedAsBase_async(
                    const ::Test::AMD_TestIntf_unknownPreservedAsBasePtr&,
                    const ::Ice::Current&);

    virtual void unknownPreservedAsKnownPreserved_async(
                    const ::Test::AMD_TestIntf_unknownPreservedAsKnownPreservedPtr&,
                    const ::Ice::Current&);

    virtual void relayUnknownPreservedAsBase_async(
                    const ::Test::AMD_TestIntf_relayUnknownPreservedAsBasePtr&,
                    const ::Test::RelayPrx&,
                    const ::Ice::Current&);

    virtual void relayUnknownPreservedAsKnownPreserved_async(
                    const ::Test::AMD_TestIntf_relayUnknownPreservedAsKnownPreservedPtr&,
                    const ::Test::RelayPrx&,
                    const ::Ice::Current&);

    virtual void shutdown_async(const ::Test::AMD_TestIntf_shutdownPtr&, const ::Ice::Current&);
#endif
};

#endif
