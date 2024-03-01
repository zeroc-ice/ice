//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TESTAMDI_H
#define TESTAMDI_H

#include <ServerPrivateAMD.h>

class TestI final : public Test::TestIntf
{
public:

    TestI();

    void baseAsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void unknownDerivedAsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void knownDerivedAsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void knownDerivedAsKnownDerivedAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void unknownIntermediateAsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void knownIntermediateAsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void knownMostDerivedAsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void knownIntermediateAsKnownIntermediateAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void knownMostDerivedAsKnownIntermediateAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void knownMostDerivedAsKnownMostDerivedAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void unknownMostDerived1AsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void unknownMostDerived1AsKnownIntermediateAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void unknownMostDerived2AsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void unknownMostDerived2AsBaseCompactAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void knownPreservedAsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void knownPreservedAsKnownPreservedAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void relayKnownPreservedAsBaseAsync(
        std::optional<Test::RelayPrx>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void relayKnownPreservedAsKnownPreservedAsync(
        std::optional<Test::RelayPrx>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void unknownPreservedAsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void unknownPreservedAsKnownPreservedAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void relayUnknownPreservedAsBaseAsync(
        std::optional<Test::RelayPrx>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void relayUnknownPreservedAsKnownPreservedAsync(
        std::optional<Test::RelayPrx>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;

    void shutdownAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const ::Ice::Current&) final;
};

#endif
