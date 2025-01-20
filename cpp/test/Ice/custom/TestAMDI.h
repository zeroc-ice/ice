// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "TestAMD.h"

class TestIntfI : public virtual Test::TestIntf
{
public:
    void opShortArrayAsync(
        std::pair<const std::int16_t*, const std::int16_t*>,
        std::function<void(
            std::pair<const std::int16_t*, const std::int16_t*>,
            std::pair<const std::int16_t*, const std::int16_t*>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opDoubleArrayAsync(
        bool,
        std::pair<const double*, const double*>,
        std::function<void(const Test::DoubleSeq&, const Test::DoubleSeq&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opBoolArrayAsync(
        std::pair<const bool*, const bool*>,
        std::function<void(const Test::BoolSeq&, const Test::BoolSeq&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opByteArrayAsync(
        std::pair<const std::byte*, const std::byte*>,
        std::function<
            void(std::pair<const std::byte*, const std::byte*>, std::pair<const std::byte*, const std::byte*>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opVariableArrayAsync(
        std::pair<const Test::Variable*, const Test::Variable*>,
        std::function<void(const Test::VariableList&, const Test::VariableList&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opBoolSeqAsync(
        std::deque<bool>,
        std::function<void(const std::deque<bool>&, const std::deque<bool>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opBoolListAsync(
        Test::BoolList,
        std::function<void(const Test::BoolList&, const Test::BoolList&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opByteSeqAsync(
        std::deque<std::byte>,
        std::function<void(const std::deque<std::byte>&, const std::deque<std::byte>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opByteListAsync(
        Test::ByteList,
        std::function<void(const Test::ByteList&, const Test::ByteList&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opMyByteSeqAsync(
        MyByteSeq,
        std::function<void(const MyByteSeq&, const MyByteSeq&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opStringSeqAsync(
        std::deque<std::string>,
        std::function<void(const std::deque<std::string>&, const std::deque<std::string>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opStringListAsync(
        Test::StringList,
        std::function<void(const Test::StringList&, const Test::StringList&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opFixedSeqAsync(
        std::deque<Test::Fixed>,
        std::function<void(const std::deque<Test::Fixed>&, const std::deque<Test::Fixed>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opFixedListAsync(
        Test::FixedList,
        std::function<void(const Test::FixedList&, const Test::FixedList&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opVariableSeqAsync(
        std::deque<Test::Variable>,
        std::function<void(const std::deque<Test::Variable>&, const std::deque<Test::Variable>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opVariableListAsync(
        Test::VariableList,
        std::function<void(const Test::VariableList&, const Test::VariableList&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opStringStringDictSeqAsync(
        std::deque<Test::StringStringDict>,
        std::function<void(const std::deque<Test::StringStringDict>&, const std::deque<Test::StringStringDict>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opStringStringDictListAsync(
        Test::StringStringDictList,
        std::function<void(const Test::StringStringDictList&, const Test::StringStringDictList&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opESeqAsync(
        std::deque<Test::E>,
        std::function<void(const std::deque<Test::E>&, const std::deque<Test::E>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opEListAsync(
        Test::EList,
        std::function<void(const Test::EList&, const Test::EList&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opDPrxSeqAsync(
        std::deque<std::optional<Test::DPrx>>,
        std::function<void(const std::deque<std::optional<Test::DPrx>>&, const std::deque<std::optional<Test::DPrx>>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opDPrxListAsync(
        Test::DPrxList,
        std::function<void(const Test::DPrxList&, const Test::DPrxList&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opCSeqAsync(
        std::deque<std::shared_ptr<Test::C>>,
        std::function<void(const std::deque<std::shared_ptr<Test::C>>&, const std::deque<std::shared_ptr<Test::C>>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opCListAsync(
        Test::CList,
        std::function<void(const Test::CList&, const Test::CList&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opOutArrayByteSeqAsync(
        Test::ByteSeq,
        std::function<void(std::pair<const std::byte*, const std::byte*>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opIntStringDictAsync(
        Test::IntStringDict,
        std::function<void(const Test::IntStringDict&, const Test::IntStringDict&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opVarDictAsync(
        Test::CustomMap<std::string, std::int32_t>,
        std::function<void(
            const Test::CustomMap<std::int64_t, std::int64_t>&,
            const Test::CustomMap<std::string, std::int32_t>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opShortBufferAsync(
        Test::ShortBuffer,
        std::function<void(const Test::ShortBuffer&, const Test::ShortBuffer&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opBoolBufferAsync(
        Test::CustomBuffer<bool>,
        std::function<void(const Test::CustomBuffer<bool>&, const Test::CustomBuffer<bool>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void opBufferStructAsync(
        Test::BufferStruct,
        std::function<void(const Test::BufferStruct&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void shutdownAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) override;
};

#endif
