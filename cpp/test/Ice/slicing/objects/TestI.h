// Copyright (c) ZeroC, Inc.

#ifndef TESTI_H
#define TESTI_H

#include "ServerPrivate.h"

class TestI : public virtual Test::TestIntf
{
public:
    TestI();
    ~TestI() override;

    Ice::ValuePtr SBaseAsObject(const Ice::Current&) override;
    ::Test::SBasePtr SBaseAsSBase(const Ice::Current&) override;
    ::Test::SBasePtr SBSKnownDerivedAsSBase(const Ice::Current&) override;
    ::Test::SBSKnownDerivedPtr SBSKnownDerivedAsSBSKnownDerived(const Ice::Current&) override;

    ::Test::SBasePtr SBSUnknownDerivedAsSBase(const Ice::Current&) override;

    ::Test::SBasePtr SBSUnknownDerivedAsSBaseCompact(const Ice::Current&) override;

    Ice::ValuePtr SUnknownAsObject(const Ice::Current&) override;
    void checkSUnknown(Ice::ValuePtr object, const Ice::Current&) override;

    ::Test::BPtr oneElementCycle(const Ice::Current&) override;
    ::Test::BPtr twoElementCycle(const Ice::Current&) override;

    ::Test::BPtr D1AsB(const Ice::Current&) override;
    ::Test::D1Ptr D1AsD1(const Ice::Current&) override;
    ::Test::BPtr D2AsB(const Ice::Current&) override;

    void paramTest1(::Test::BPtr&, ::Test::BPtr&, const Ice::Current&) override;
    void paramTest2(::Test::BPtr&, ::Test::BPtr&, const Ice::Current&) override;
    ::Test::BPtr paramTest3(::Test::BPtr&, ::Test::BPtr&, const Ice::Current&) override;
    ::Test::BPtr paramTest4(::Test::BPtr&, const Ice::Current&) override;

    ::Test::BPtr returnTest1(::Test::BPtr&, ::Test::BPtr&, const Ice::Current&) override;
    ::Test::BPtr returnTest2(::Test::BPtr&, ::Test::BPtr&, const Ice::Current&) override;
    ::Test::BPtr returnTest3(::Test::BPtr, ::Test::BPtr, const Ice::Current&) override;

    ::Test::SS3 sequenceTest(::Test::SS1Ptr, ::Test::SS2Ptr, const Ice::Current&) override;

    ::Test::BDict dictionaryTest(::Test::BDict, ::Test::BDict&, const Ice::Current&) override;

    ::Test::PBasePtr exchangePBase(::Test::PBasePtr, const Ice::Current&) override;

    ::Test::PreservedPtr PBSUnknownAsPreserved(const Ice::Current&) override;
    void checkPBSUnknown(::Test::PreservedPtr, const Ice::Current&) override;

    void PBSUnknownAsPreservedWithGraphAsync(
        std::function<void(const std::shared_ptr<Test::Preserved>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;
    void checkPBSUnknownWithGraph(::Test::PreservedPtr, const Ice::Current&) override;

    void PBSUnknown2AsPreservedWithGraphAsync(
        std::function<void(const std::shared_ptr<Test::Preserved>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;
    void checkPBSUnknown2WithGraph(::Test::PreservedPtr, const Ice::Current&) override;

    ::Test::PNodePtr exchangePNode(::Test::PNodePtr, const Ice::Current&) override;

    void throwBaseAsBase(const Ice::Current&) override;
    void throwDerivedAsBase(const Ice::Current&) override;
    void throwDerivedAsDerived(const Ice::Current&) override;
    void throwUnknownDerivedAsBase(const Ice::Current&) override;

    void useForward(::Test::ForwardPtr&, const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;

private:
    std::vector<Ice::ValuePtr> _values;
};

#endif
