// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestIntfI final : public Test::TestIntf
{
public:
    Test::ShortSeq
    opShortArray(std::pair<const std::int16_t*, const std::int16_t*>, Test::ShortSeq&, const Ice::Current&) final;

    Test::DoubleSeq
    opDoubleArray(bool, std::pair<const double*, const double*>, Test::DoubleSeq&, const Ice::Current&) final;

    Test::BoolSeq opBoolArray(std::pair<const bool*, const bool*>, Test::BoolSeq&, const Ice::Current&) final;

    Test::ByteList
    opByteArray(std::pair<const std::byte*, const std::byte*>, Test::ByteList&, const Ice::Current&) final;

    Test::VariableList opVariableArray(
        std::pair<const Test::Variable*, const Test::Variable*>,
        Test::VariableList&,
        const Ice::Current&) final;

    std::deque<bool> opBoolSeq(std::deque<bool>, std::deque<bool>&, const Ice::Current&) final;

    std::list<bool> opBoolList(std::list<bool>, std::list<bool>&, const Ice::Current&) final;

    Test::BoolDequeList opBoolDequeList(Test::BoolDequeList, Test::BoolDequeList&, const Ice::Current&) final;

    Test::BoolDequeList opBoolDequeListArray(
        std::pair<const std::deque<bool>*, const std::deque<bool>*>,
        Test::BoolDequeList&,
        const Ice::Current&) final;

    std::deque<std::byte> opByteSeq(std::deque<std::byte>, std::deque<std::byte>&, const Ice::Current&) final;

    std::list<std::byte> opByteList(std::list<std::byte>, std::list<std::byte>&, const Ice::Current&) final;

    MyByteSeq opMyByteSeq(MyByteSeq, MyByteSeq&, const Ice::Current&) final;

    std::deque<std::string> opStringSeq(std::deque<std::string>, std::deque<std::string>&, const Ice::Current&) final;

    std::list<std::string> opStringList(std::list<std::string>, std::list<std::string>&, const Ice::Current&) final;

    std::deque<Test::Fixed> opFixedSeq(std::deque<Test::Fixed>, std::deque<Test::Fixed>&, const Ice::Current&) final;

    std::list<Test::Fixed> opFixedList(std::list<Test::Fixed>, std::list<Test::Fixed>&, const Ice::Current&) final;

    std::deque<Test::Variable>
    opVariableSeq(std::deque<Test::Variable>, std::deque<Test::Variable>&, const Ice::Current&) final;

    std::list<Test::Variable>
    opVariableList(std::list<Test::Variable>, std::list<Test::Variable>&, const Ice::Current&) final;

    std::deque<Test::StringStringDict> opStringStringDictSeq(
        std::deque<Test::StringStringDict>,
        std::deque<Test::StringStringDict>&,
        const Ice::Current&) final;

    std::list<Test::StringStringDict> opStringStringDictList(
        std::list<Test::StringStringDict>,
        std::list<Test::StringStringDict>&,
        const Ice::Current&) final;

    std::deque<Test::E> opESeq(std::deque<Test::E>, std::deque<Test::E>&, const Ice::Current&) final;

    std::list<Test::E> opEList(std::list<Test::E>, std::list<Test::E>&, const Ice::Current&) final;

    std::deque<std::optional<Test::DPrx>>
    opDPrxSeq(std::deque<std::optional<Test::DPrx>>, std::deque<std::optional<Test::DPrx>>&, const Ice::Current&) final;

    Test::DPrxList opDPrxList(Test::DPrxList, Test::DPrxList&, const Ice::Current&) final;

    std::deque<Test::CPtr> opCSeq(std::deque<Test::CPtr>, std::deque<Test::CPtr>&, const Ice::Current&) final;

    std::list<Test::CPtr> opCList(std::list<Test::CPtr>, std::list<Test::CPtr>&, const Ice::Current&) final;

    void opOutArrayByteSeq(Test::ByteSeq, Test::ByteSeq&, const Ice::Current&) final;

    Test::IntStringDict opIntStringDict(Test::IntStringDict, Test::IntStringDict&, const Ice::Current&) final;

    Test::CustomMap<std::int64_t, std::int64_t> opVarDict(
        Test::CustomMap<std::string, std::int32_t>,
        Test::CustomMap<std::string, std::int32_t>&,
        const Ice::Current&) final;

    Test::ShortBuffer opShortBuffer(Test::ShortBuffer, Test::ShortBuffer&, const Ice::Current&) override;

    Test::CustomBuffer<bool>
    opBoolBuffer(Test::CustomBuffer<bool>, Test::CustomBuffer<bool>&, const Ice::Current&) override;

    Test::BufferStruct opBufferStruct(Test::BufferStruct, const Ice::Current&) override;

    void shutdown(const Ice::Current&) final;
};

#endif
