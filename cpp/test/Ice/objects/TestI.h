// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Forward.h"
#include "Test.h"

class BI final : public Test::B
{
public:
    void ice_preMarshal() final;
    void ice_postUnmarshal() final;
};

class CI final : public Test::C
{
public:
    void ice_preMarshal() final;
    void ice_postUnmarshal() final;
};

class DI : public Test::D
{
public:
    void ice_preMarshal() final;
    void ice_postUnmarshal() final;
};

class CustomSliceLoader final : public Ice::SliceLoader
{
public:
    [[nodiscard]] Ice::ValuePtr newClassInstance(std::string_view typeId) const final
    {
        if (typeId == "::Test::B")
        {
            return std::make_shared<BI>();
        }
        else if (typeId == "::Test::C")
        {
            return std::make_shared<CI>();
        }
        else if (typeId == "::Test::D")
        {
            return std::make_shared<DI>();
        }
        return nullptr;
    }
};

class InitialI final : public Test::Initial
{
public:
    InitialI(Ice::ObjectAdapterPtr);
    ~InitialI() override;

    void shutdown(const Ice::Current&) final;
    Test::BPtr getB1(const Ice::Current&) final;
    Test::BPtr getB2(const Ice::Current&) final;
    Test::CPtr getC(const Ice::Current&) final;
    Test::DPtr getD(const Ice::Current&) final;

    void setRecursive(Test::RecursivePtr, const Ice::Current&) final;

    void setCycle(Test::RecursivePtr, const Ice::Current&) final;
    bool acceptsClassCycles(const Ice::Current&) final;

    GetMBMarshaledResult getMB(const Ice::Current&) final;
    void getAMDMBAsync(
        std::function<void(GetAMDMBMarshaledResult)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void getAll(Test::BPtr&, Test::BPtr&, Test::CPtr&, Test::DPtr&, const Ice::Current&) final;

    Test::CppClassPtr opRenamedClass(Test::CppClassPtr r, const Ice::Current&) final;

    Test::KPtr getK(const Ice::Current&) final;

    Ice::ValuePtr opValue(Ice::ValuePtr, Ice::ValuePtr&, const Ice::Current&) final;
    Test::ValueSeq opValueSeq(Test::ValueSeq, Test::ValueSeq&, const Ice::Current&) final;
    Test::ValueMap opValueMap(Test::ValueMap, Test::ValueMap&, const Ice::Current&) final;

    Test::D1Ptr getD1(Test::D1Ptr, const Ice::Current&) final;
    void throwEDerived(const Ice::Current&) final;

    void setG(Test::GPtr, const Ice::Current&) final;

    Test::BaseSeq opBaseSeq(Test::BaseSeq, Test::BaseSeq&, const Ice::Current&) final;

    Test::CompactPtr getCompact(const Ice::Current&) final;

    Test::Inner::APtr getInnerA(const Ice::Current&) final;
    Test::Inner::Sub::APtr getInnerSubA(const Ice::Current&) final;

    void throwInnerEx(const Ice::Current&) final;
    void throwInnerSubEx(const Ice::Current&) final;

    Test::MPtr opM(Test::MPtr, Test::MPtr&, const Ice::Current&) final;

    Test::F1Ptr opF1(Test::F1Ptr, Test::F1Ptr&, const Ice::Current&) final;
    std::optional<Test::F2Prx> opF2(std::optional<Test::F2Prx>, std::optional<Test::F2Prx>&, const Ice::Current&) final;
    Test::F3Ptr opF3(Test::F3Ptr, Test::F3Ptr&, const Ice::Current&) final;
    bool hasF3(const Ice::Current&) final;

private:
    Ice::ObjectAdapterPtr _adapter;
    Test::BPtr _b1;
    Test::BPtr _b2;
    Test::CPtr _c;
    Test::DPtr _d;
};

class UnexpectedObjectExceptionTestI : public Ice::Blobject
{
public:
    bool ice_invoke(std::vector<std::byte>, std::vector<std::byte>&, const Ice::Current&) final;
};
using UnexpectedObjectExceptionTestIPtr = std::shared_ptr<UnexpectedObjectExceptionTestI>;

class TestIntfI : public Test::TestIntf
{
public:
    Test::BasePtr opDerived(const Ice::Current&) final;
    void throwDerived(const Ice::Current&) final;
};

class F2I : public Test::F2
{
public:
    void op(const Ice::Current&) override {}
};

#endif
