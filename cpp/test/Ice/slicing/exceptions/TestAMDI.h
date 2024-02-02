//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TESTAMDI_H
#define TESTAMDI_H

#include <ServerPrivateAMD.h>

class TestI : public virtual Test::TestIntf
{
public:

    TestI();
    virtual void baseAsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void unknownDerivedAsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void knownDerivedAsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void knownDerivedAsKnownDerivedAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void unknownIntermediateAsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void knownIntermediateAsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void knownMostDerivedAsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void knownIntermediateAsKnownIntermediateAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void knownMostDerivedAsKnownIntermediateAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void knownMostDerivedAsKnownMostDerivedAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void unknownMostDerived1AsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void unknownMostDerived1AsKnownIntermediateAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void unknownMostDerived2AsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void unknownMostDerived2AsBaseCompactAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void knownPreservedAsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void knownPreservedAsKnownPreservedAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void relayKnownPreservedAsBaseAsync(
        std::shared_ptr<::Test::RelayPrx>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void relayKnownPreservedAsKnownPreservedAsync(
        std::shared_ptr<::Test::RelayPrx>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void unknownPreservedAsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void unknownPreservedAsKnownPreservedAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void relayUnknownPreservedAsBaseAsync(
        std::shared_ptr<::Test::RelayPrx>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void relayUnknownPreservedAsKnownPreservedAsync(
        std::shared_ptr<::Test::RelayPrx>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);

    virtual void shutdownAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&);
};

#endif
