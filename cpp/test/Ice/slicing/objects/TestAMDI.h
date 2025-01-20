// Copyright (c) ZeroC, Inc.

#ifndef TESTI_H
#define TESTI_H

#include "ServerPrivateAMD.h"

class TestI : public virtual ::Test::TestIntf
{
public:
    TestI();
    void SBaseAsObjectAsync(
        std::function<void(const Ice::ValuePtr&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void SBaseAsSBaseAsync(
        std::function<void(const std::shared_ptr<Test::SBase>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void SBSKnownDerivedAsSBaseAsync(
        std::function<void(const std::shared_ptr<Test::SBase>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void SBSKnownDerivedAsSBSKnownDerivedAsync(
        std::function<void(const std::shared_ptr<Test::SBSKnownDerived>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void SBSUnknownDerivedAsSBaseAsync(
        std::function<void(const std::shared_ptr<Test::SBase>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void SBSUnknownDerivedAsSBaseCompactAsync(
        std::function<void(const std::shared_ptr<Test::SBase>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void SUnknownAsObjectAsync(
        std::function<void(const Ice::ValuePtr&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void checkSUnknownAsync(
        Ice::ValuePtr,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void oneElementCycleAsync(
        std::function<void(const std::shared_ptr<Test::B>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void twoElementCycleAsync(
        std::function<void(const std::shared_ptr<Test::B>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void D1AsBAsync(
        std::function<void(const std::shared_ptr<Test::B>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void D1AsD1Async(
        std::function<void(const std::shared_ptr<Test::D1>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void D2AsBAsync(
        std::function<void(const std::shared_ptr<Test::B>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void paramTest1Async(
        std::function<void(const std::shared_ptr<Test::B>&, const std::shared_ptr<Test::B>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void paramTest2Async(
        std::function<void(const std::shared_ptr<Test::B>&, const std::shared_ptr<Test::B>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void paramTest3Async(
        std::function<
            void(const std::shared_ptr<Test::B>&, const std::shared_ptr<Test::B>&, const std::shared_ptr<Test::B>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void paramTest4Async(
        std::function<void(const std::shared_ptr<Test::B>&, const std::shared_ptr<Test::B>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void returnTest1Async(
        std::function<
            void(const std::shared_ptr<Test::B>&, const std::shared_ptr<Test::B>&, const std::shared_ptr<Test::B>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void returnTest2Async(
        std::function<
            void(const std::shared_ptr<Test::B>&, const std::shared_ptr<Test::B>&, const std::shared_ptr<Test::B>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void returnTest3Async(
        std::shared_ptr<::Test::B>,
        std::shared_ptr<::Test::B>,
        std::function<void(const std::shared_ptr<Test::B>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void sequenceTestAsync(
        std::shared_ptr<::Test::SS1>,
        std::shared_ptr<::Test::SS2>,
        std::function<void(const ::Test::SS3&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void dictionaryTestAsync(
        Test::BDict,
        std::function<void(const ::Test::BDict&, const ::Test::BDict&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void exchangePBaseAsync(
        std::shared_ptr<::Test::PBase>,
        std::function<void(const std::shared_ptr<::Test::PBase>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void PBSUnknownAsPreservedAsync(
        std::function<void(const std::shared_ptr<::Test::Preserved>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void checkPBSUnknownAsync(
        std::shared_ptr<::Test::Preserved>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void PBSUnknownAsPreservedWithGraphAsync(
        std::function<void(const std::shared_ptr<::Test::Preserved>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void checkPBSUnknownWithGraphAsync(
        std::shared_ptr<::Test::Preserved>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void PBSUnknown2AsPreservedWithGraphAsync(
        std::function<void(const std::shared_ptr<::Test::Preserved>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void checkPBSUnknown2WithGraphAsync(
        std::shared_ptr<::Test::Preserved>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void exchangePNodeAsync(
        std::shared_ptr<::Test::PNode>,
        std::function<void(const std::shared_ptr<::Test::PNode>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void
    throwBaseAsBaseAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) override;

    void throwDerivedAsBaseAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void throwDerivedAsDerivedAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void throwUnknownDerivedAsBaseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void useForwardAsync(
        std::function<void(const std::shared_ptr<::Test::Forward>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void shutdownAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) override;
};

#endif
