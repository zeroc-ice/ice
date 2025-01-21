// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class InitialI : public Test::Initial
{
public:
    InitialI();

    void shutdown(const Ice::Current&) override;
    PingPongMarshaledResult pingPong(Ice::ValuePtr, const Ice::Current&) override;

    void opOptionalException(std::optional<std::int32_t>, std::optional<std::string>, const Ice::Current&) override;

    void opDerivedException(std::optional<std::int32_t>, std::optional<std::string>, const Ice::Current&) override;

    void opRequiredException(std::optional<std::int32_t>, std::optional<std::string>, const Ice::Current&) override;

    std::optional<std::uint8_t>
    opByte(std::optional<std::uint8_t>, std::optional<std::uint8_t>&, const Ice::Current&) override;

    std::optional<bool> opBool(std::optional<bool>, std::optional<bool>&, const Ice::Current&) override;

    std::optional<std::int16_t>
    opShort(std::optional<std::int16_t>, std::optional<std::int16_t>&, const Ice::Current&) override;

    std::optional<std::int32_t>
    opInt(std::optional<std::int32_t>, std::optional<std::int32_t>&, const Ice::Current&) override;

    std::optional<std::int64_t>
    opLong(std::optional<std::int64_t>, std::optional<std::int64_t>&, const Ice::Current&) override;

    std::optional<float> opFloat(std::optional<float>, std::optional<float>&, const Ice::Current&) override;

    std::optional<double> opDouble(std::optional<double>, std::optional<double>&, const Ice::Current&) override;

    std::optional<std::string>
    opString(std::optional<std::string>, std::optional<std::string>&, const Ice::Current&) override;

    std::optional<Test::MyEnum>
    opMyEnum(std::optional<Test::MyEnum>, std::optional<Test::MyEnum>&, const Ice::Current&) override;

    std::optional<Test::SmallStruct>
    opSmallStruct(std::optional<Test::SmallStruct>, std::optional<Test::SmallStruct>&, const Ice::Current&) override;

    std::optional<Test::FixedStruct>
    opFixedStruct(std::optional<Test::FixedStruct>, std::optional<Test::FixedStruct>&, const Ice::Current&) override;

    std::optional<Test::VarStruct>
    opVarStruct(std::optional<Test::VarStruct>, std::optional<Test::VarStruct>&, const Ice::Current&) override;

    std::optional<Test::MyInterfacePrx> opMyInterfaceProxy(
        std::optional<Test::MyInterfacePrx>,
        std::optional<Test::MyInterfacePrx>&,
        const Ice::Current&) override;

    Test::OneOptionalPtr opOneOptional(Test::OneOptionalPtr, Test::OneOptionalPtr&, const Ice::Current&) override;

    std::optional<::Test::ByteSeq> opByteSeq(
        std::optional<std::pair<const std::byte*, const std::byte*>>,
        std::optional<::Test::ByteSeq>&,
        const Ice::Current&) override;

    std::optional<::Test::BoolSeq> opBoolSeq(
        std::optional<std::pair<const bool*, const bool*>>,
        std::optional<::Test::BoolSeq>&,
        const Ice::Current&) override;

    std::optional<::Test::ShortSeq> opShortSeq(
        std::optional<std::pair<const std::int16_t*, const std::int16_t*>>,
        std::optional<::Test::ShortSeq>&,
        const Ice::Current&) override;

    std::optional<::Test::IntSeq> opIntSeq(
        std::optional<std::pair<const std::int32_t*, const std::int32_t*>>,
        std::optional<::Test::IntSeq>&,
        const Ice::Current&) override;

    std::optional<::Test::LongSeq> opLongSeq(
        std::optional<std::pair<const std::int64_t*, const std::int64_t*>>,
        std::optional<::Test::LongSeq>&,
        const Ice::Current&) override;

    std::optional<::Test::FloatSeq> opFloatSeq(
        std::optional<std::pair<const float*, const float*>>,
        std::optional<::Test::FloatSeq>&,
        const Ice::Current&) override;

    std::optional<::Test::DoubleSeq> opDoubleSeq(
        std::optional<std::pair<const double*, const double*>>,
        std::optional<::Test::DoubleSeq>&,
        const Ice::Current&) override;

    std::optional<::Test::StringSeq>
    opStringSeq(std::optional<::Test::StringSeq>, std::optional<::Test::StringSeq>&, const Ice::Current&) override;

    std::optional<::Test::SmallStructSeq> opSmallStructSeq(
        std::optional<std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>,
        std::optional<::Test::SmallStructSeq>&,
        const Ice::Current&) override;

    std::optional<::Test::SmallStructList> opSmallStructList(
        std::optional<std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>,
        std::optional<::Test::SmallStructList>&,
        const Ice::Current&) override;

    std::optional<::Test::FixedStructSeq> opFixedStructSeq(
        std::optional<std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>,
        std::optional<::Test::FixedStructSeq>&,
        const Ice::Current&) override;

    std::optional<::Test::FixedStructList> opFixedStructList(
        std::optional<std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>,
        std::optional<::Test::FixedStructList>&,
        const Ice::Current&) override;

    std::optional<::Test::VarStructSeq> opVarStructSeq(
        std::optional<::Test::VarStructSeq>,
        std::optional<::Test::VarStructSeq>&,
        const Ice::Current&) override;

    std::optional<::Test::Serializable> opSerializable(
        std::optional<::Test::Serializable>,
        std::optional<::Test::Serializable>&,
        const Ice::Current&) override;

    std::optional<::Test::IntIntDict>
    opIntIntDict(std::optional<::Test::IntIntDict>, std::optional<::Test::IntIntDict>&, const Ice::Current&) override;

    std::optional<::Test::StringIntDict> opStringIntDict(
        std::optional<::Test::StringIntDict>,
        std::optional<::Test::StringIntDict>&,
        const Ice::Current&) override;

    void opClassAndUnknownOptional(Test::APtr, const Ice::Current&) override;

    ::Test::GPtr opG(::Test::GPtr g, const Ice::Current&) override;

    void opVoid(const Ice::Current&) override;

    OpMStruct1MarshaledResult opMStruct1(const Ice::Current&) override;

    OpMStruct2MarshaledResult opMStruct2(std::optional<Test::SmallStruct>, const Ice::Current&) override;

    OpMSeq1MarshaledResult opMSeq1(const Ice::Current&) override;

    OpMSeq2MarshaledResult opMSeq2(std::optional<Test::StringSeq>, const Ice::Current&) override;

    OpMDict1MarshaledResult opMDict1(const Ice::Current&) override;

    OpMDict2MarshaledResult opMDict2(std::optional<Test::StringIntDict>, const Ice::Current&) override;

    bool supportsJavaSerializable(const Ice::Current&) override;
};

#endif
