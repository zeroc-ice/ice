//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class InitialI : public Test::Initial
{
public:
    InitialI();

    virtual void shutdown(const Ice::Current&);
    virtual PingPongMarshaledResult pingPong(Ice::ValuePtr, const Ice::Current&);

    virtual void opOptionalException(std::optional<std::int32_t>, std::optional<std::string>, const Ice::Current&);

    virtual void opDerivedException(std::optional<std::int32_t>, std::optional<std::string>, const Ice::Current&);

    virtual void opRequiredException(std::optional<std::int32_t>, std::optional<std::string>, const Ice::Current&);

    virtual std::optional<std::uint8_t>
    opByte(std::optional<std::uint8_t>, std::optional<std::uint8_t>&, const Ice::Current&);

    virtual std::optional<bool> opBool(std::optional<bool>, std::optional<bool>&, const Ice::Current&);

    virtual std::optional<std::int16_t>
    opShort(std::optional<std::int16_t>, std::optional<std::int16_t>&, const Ice::Current&);

    virtual std::optional<std::int32_t>
    opInt(std::optional<std::int32_t>, std::optional<std::int32_t>&, const Ice::Current&);

    virtual std::optional<std::int64_t>
    opLong(std::optional<std::int64_t>, std::optional<std::int64_t>&, const Ice::Current&);

    virtual std::optional<float> opFloat(std::optional<float>, std::optional<float>&, const Ice::Current&);

    virtual std::optional<double> opDouble(std::optional<double>, std::optional<double>&, const Ice::Current&);

    virtual std::optional<std::string>
    opString(std::optional<std::string>, std::optional<std::string>&, const Ice::Current&);

    virtual std::optional<Test::MyEnum>
    opMyEnum(std::optional<Test::MyEnum>, std::optional<Test::MyEnum>&, const Ice::Current&);

    virtual std::optional<Test::SmallStruct>
    opSmallStruct(std::optional<Test::SmallStruct>, std::optional<Test::SmallStruct>&, const Ice::Current&);

    virtual std::optional<Test::FixedStruct>
    opFixedStruct(std::optional<Test::FixedStruct>, std::optional<Test::FixedStruct>&, const Ice::Current&);

    virtual std::optional<Test::VarStruct>
    opVarStruct(std::optional<Test::VarStruct>, std::optional<Test::VarStruct>&, const Ice::Current&);

    virtual std::optional<Test::MyInterfacePrx>
    opMyInterfaceProxy(std::optional<Test::MyInterfacePrx>, std::optional<Test::MyInterfacePrx>&, const Ice::Current&);

    virtual Test::OneOptionalPtr opOneOptional(Test::OneOptionalPtr, Test::OneOptionalPtr&, const Ice::Current&);

    virtual std::optional<::Test::ByteSeq> opByteSeq(
        std::optional<std::pair<const std::byte*, const std::byte*>>,
        std::optional<::Test::ByteSeq>&,
        const Ice::Current&);

    virtual std::optional<::Test::BoolSeq>
    opBoolSeq(std::optional<std::pair<const bool*, const bool*>>, std::optional<::Test::BoolSeq>&, const Ice::Current&);

    virtual std::optional<::Test::ShortSeq> opShortSeq(
        std::optional<std::pair<const std::int16_t*, const std::int16_t*>>,
        std::optional<::Test::ShortSeq>&,
        const Ice::Current&);

    virtual std::optional<::Test::IntSeq> opIntSeq(
        std::optional<std::pair<const std::int32_t*, const std::int32_t*>>,
        std::optional<::Test::IntSeq>&,
        const Ice::Current&);

    virtual std::optional<::Test::LongSeq> opLongSeq(
        std::optional<std::pair<const std::int64_t*, const std::int64_t*>>,
        std::optional<::Test::LongSeq>&,
        const Ice::Current&);

    virtual std::optional<::Test::FloatSeq> opFloatSeq(
        std::optional<std::pair<const float*, const float*>>,
        std::optional<::Test::FloatSeq>&,
        const Ice::Current&);

    virtual std::optional<::Test::DoubleSeq> opDoubleSeq(
        std::optional<std::pair<const double*, const double*>>,
        std::optional<::Test::DoubleSeq>&,
        const Ice::Current&);

    virtual std::optional<::Test::StringSeq>
    opStringSeq(std::optional<::Test::StringSeq>, std::optional<::Test::StringSeq>&, const Ice::Current&);

    virtual std::optional<::Test::SmallStructSeq> opSmallStructSeq(
        std::optional<std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>,
        std::optional<::Test::SmallStructSeq>&,
        const Ice::Current&);

    virtual std::optional<::Test::SmallStructList> opSmallStructList(
        std::optional<std::pair<const ::Test::SmallStruct*, const ::Test::SmallStruct*>>,
        std::optional<::Test::SmallStructList>&,
        const Ice::Current&);

    virtual std::optional<::Test::FixedStructSeq> opFixedStructSeq(
        std::optional<std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>,
        std::optional<::Test::FixedStructSeq>&,
        const Ice::Current&);

    virtual std::optional<::Test::FixedStructList> opFixedStructList(
        std::optional<std::pair<const ::Test::FixedStruct*, const ::Test::FixedStruct*>>,
        std::optional<::Test::FixedStructList>&,
        const Ice::Current&);

    virtual std::optional<::Test::VarStructSeq>
    opVarStructSeq(std::optional<::Test::VarStructSeq>, std::optional<::Test::VarStructSeq>&, const Ice::Current&);

    virtual std::optional<::Test::Serializable>
    opSerializable(std::optional<::Test::Serializable>, std::optional<::Test::Serializable>&, const Ice::Current&);

    virtual std::optional<::Test::IntIntDict>
    opIntIntDict(std::optional<::Test::IntIntDict>, std::optional<::Test::IntIntDict>&, const Ice::Current&);

    virtual std::optional<::Test::StringIntDict>
    opStringIntDict(std::optional<::Test::StringIntDict>, std::optional<::Test::StringIntDict>&, const Ice::Current&);

    virtual void opClassAndUnknownOptional(Test::APtr, const Ice::Current&);

    virtual ::Test::GPtr opG(::Test::GPtr g, const Ice::Current&);

    virtual void opVoid(const Ice::Current&);

    virtual OpMStruct1MarshaledResult opMStruct1(const Ice::Current&);

    virtual OpMStruct2MarshaledResult opMStruct2(std::optional<Test::SmallStruct>, const Ice::Current&);

    virtual OpMSeq1MarshaledResult opMSeq1(const Ice::Current&);

    virtual OpMSeq2MarshaledResult opMSeq2(std::optional<Test::StringSeq>, const Ice::Current&);

    virtual OpMDict1MarshaledResult opMDict1(const Ice::Current&);

    virtual OpMDict2MarshaledResult opMDict2(std::optional<Test::StringIntDict>, const Ice::Current&);

    virtual bool supportsJavaSerializable(const Ice::Current&);
};

#endif
