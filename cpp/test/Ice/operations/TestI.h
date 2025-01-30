// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class MyDerivedClassI final : public Test::MyDerivedClass
{
public:
    MyDerivedClassI();

    void shutdown(const Ice::Current&) final;

    bool supportsCompress(const Ice::Current&) final;

    void opVoid(const Ice::Current&) final;

    std::uint8_t opByte(std::uint8_t, std::uint8_t, std::uint8_t&, const Ice::Current&) final;

    bool opBool(bool, bool, bool&, const Ice::Current&) final;

    std::int64_t opShortIntLong(
        std::int16_t,
        std::int32_t,
        std::int64_t,
        std::int16_t&,
        std::int32_t&,
        std::int64_t&,
        const Ice::Current&) final;

    double opFloatDouble(float, double, float&, double&, const Ice::Current&) final;

    std::string opString(std::string, std::string, std::string&, const Ice::Current&) final;

    Test::MyEnum opMyEnum(Test::MyEnum, Test::MyEnum&, const Ice::Current&) final;

    std::optional<Test::MyClassPrx> opMyClass(
        std::optional<Test::MyClassPrx>,
        std::optional<Test::MyClassPrx>&,
        std::optional<Test::MyClassPrx>&,
        const Ice::Current&) final;

    Test::Structure opStruct(Test::Structure, Test::Structure, Test::Structure&, const Ice::Current&) final;

    Test::ByteS opByteS(Test::ByteS, Test::ByteS, Test::ByteS&, const Ice::Current&) final;

    Test::BoolS opBoolS(Test::BoolS, Test::BoolS, Test::BoolS&, const Ice::Current&) final;

    Test::LongS opShortIntLongS(
        Test::ShortS,
        Test::IntS,
        Test::LongS,
        Test::ShortS&,
        Test::IntS&,
        Test::LongS&,
        const Ice::Current&) final;

    Test::DoubleS opFloatDoubleS(Test::FloatS, Test::DoubleS, Test::FloatS&, Test::DoubleS&, const Ice::Current&) final;

    Test::StringS opStringS(Test::StringS, Test::StringS, Test::StringS&, const Ice::Current&) final;

    Test::ByteSS opByteSS(Test::ByteSS, Test::ByteSS, Test::ByteSS&, const Ice::Current&) final;

    Test::BoolSS opBoolSS(Test::BoolSS, Test::BoolSS, Test::BoolSS&, const Ice::Current&) final;

    Test::LongSS opShortIntLongSS(
        Test::ShortSS,
        Test::IntSS,
        Test::LongSS,
        Test::ShortSS&,
        Test::IntSS&,
        Test::LongSS&,
        const Ice::Current&) final;

    Test::DoubleSS
    opFloatDoubleSS(Test::FloatSS, Test::DoubleSS, Test::FloatSS&, Test::DoubleSS&, const Ice::Current&) final;

    Test::StringSS opStringSS(Test::StringSS, Test::StringSS, Test::StringSS&, const Ice::Current&) final;

    Test::StringSSS opStringSSS(Test::StringSSS, Test::StringSSS, Test::StringSSS&, const Ice::Current&) final;

    Test::ByteBoolD opByteBoolD(Test::ByteBoolD, Test::ByteBoolD, Test::ByteBoolD&, const Ice::Current&) final;

    Test::ShortIntD opShortIntD(Test::ShortIntD, Test::ShortIntD, Test::ShortIntD&, const Ice::Current&) final;

    Test::LongFloatD opLongFloatD(Test::LongFloatD, Test::LongFloatD, Test::LongFloatD&, const Ice::Current&) final;

    Test::StringStringD
    opStringStringD(Test::StringStringD, Test::StringStringD, Test::StringStringD&, const Ice::Current&) final;

    Test::StringMyEnumD
    opStringMyEnumD(Test::StringMyEnumD, Test::StringMyEnumD, Test::StringMyEnumD&, const Ice::Current&) final;

    Test::MyEnumStringD
    opMyEnumStringD(Test::MyEnumStringD, Test::MyEnumStringD, Test::MyEnumStringD&, const Ice::Current&) final;

    Test::MyStructMyEnumD
    opMyStructMyEnumD(Test::MyStructMyEnumD, Test::MyStructMyEnumD, Test::MyStructMyEnumD&, const Ice::Current&) final;

    Test::ByteBoolDS opByteBoolDS(Test::ByteBoolDS, Test::ByteBoolDS, Test::ByteBoolDS&, const Ice::Current&) final;

    Test::ShortIntDS opShortIntDS(Test::ShortIntDS, Test::ShortIntDS, Test::ShortIntDS&, const Ice::Current&) final;

    Test::LongFloatDS
    opLongFloatDS(Test::LongFloatDS, Test::LongFloatDS, Test::LongFloatDS&, const Ice::Current&) final;

    Test::StringStringDS
    opStringStringDS(Test::StringStringDS, Test::StringStringDS, Test::StringStringDS&, const Ice::Current&) final;

    Test::StringMyEnumDS
    opStringMyEnumDS(Test::StringMyEnumDS, Test::StringMyEnumDS, Test::StringMyEnumDS&, const Ice::Current&) final;

    Test::MyStructMyEnumDS
    opMyStructMyEnumDS(Test::MyStructMyEnumDS, Test::MyStructMyEnumDS, Test::MyStructMyEnumDS&, const Ice::Current&)
        final;

    Test::MyEnumStringDS
    opMyEnumStringDS(Test::MyEnumStringDS, Test::MyEnumStringDS, Test::MyEnumStringDS&, const Ice::Current&) final;

    Test::ByteByteSD opByteByteSD(Test::ByteByteSD, Test::ByteByteSD, Test::ByteByteSD&, const Ice::Current&) final;

    Test::BoolBoolSD opBoolBoolSD(Test::BoolBoolSD, Test::BoolBoolSD, Test::BoolBoolSD&, const Ice::Current&) final;

    Test::ShortShortSD
    opShortShortSD(Test::ShortShortSD, Test::ShortShortSD, Test::ShortShortSD&, const Ice::Current&) final;

    Test::IntIntSD opIntIntSD(Test::IntIntSD, Test::IntIntSD, Test::IntIntSD&, const Ice::Current&) final;

    Test::LongLongSD opLongLongSD(Test::LongLongSD, Test::LongLongSD, Test::LongLongSD&, const Ice::Current&) final;

    Test::StringFloatSD
    opStringFloatSD(Test::StringFloatSD, Test::StringFloatSD, Test::StringFloatSD&, const Ice::Current&) final;

    Test::StringDoubleSD
    opStringDoubleSD(Test::StringDoubleSD, Test::StringDoubleSD, Test::StringDoubleSD&, const Ice::Current&) final;

    Test::StringStringSD
    opStringStringSD(Test::StringStringSD, Test::StringStringSD, Test::StringStringSD&, const Ice::Current&) final;

    Test::MyEnumMyEnumSD
    opMyEnumMyEnumSD(Test::MyEnumMyEnumSD, Test::MyEnumMyEnumSD, Test::MyEnumMyEnumSD&, const Ice::Current&) final;

    Test::IntS opIntS(Test::IntS, const Ice::Current&) final;

    void opByteSOneway(Test::ByteS, const Ice::Current&) final;
    int opByteSOnewayCallCount(const Ice::Current&) final;

    Ice::Context opContext(const Ice::Current&) final;

    void opDoubleMarshaling(double, Test::DoubleS, const Ice::Current&) final;

    void opIdempotent(const Ice::Current&) final;

    void opDerived(const Ice::Current&) final;

    std::uint8_t opByte1(std::uint8_t, const Ice::Current&) final;

    std::int16_t opShort1(std::int16_t, const Ice::Current&) final;

    std::int32_t opInt1(std::int32_t, const Ice::Current&) final;

    std::int64_t opLong1(std::int64_t, const Ice::Current&) final;

    float opFloat1(float, const Ice::Current&) final;

    double opDouble1(double, const Ice::Current&) final;

    std::string opString1(std::string, const Ice::Current&) final;

    Test::StringS opStringS1(Test::StringS, const Ice::Current&) final;

    Test::ByteBoolD opByteBoolD1(Test::ByteBoolD, const Ice::Current&) final;

    Test::StringS opStringS2(Test::StringS, const Ice::Current&) final;

    Test::ByteBoolD opByteBoolD2(Test::ByteBoolD, const Ice::Current&) final;

    Test::MyStruct1 opMyStruct1(Test::MyStruct1, const Ice::Current&) final;

    Test::MyClass1Ptr opMyClass1(Test::MyClass1Ptr, const Ice::Current&) final;

    Test::StringS opStringLiterals(const Ice::Current&) final;

    Test::WStringS opWStringLiterals(const Ice::Current&) final;

    OpMStruct1MarshaledResult opMStruct1(const Ice::Current&) final;

    OpMStruct2MarshaledResult opMStruct2(Test::Structure, const Ice::Current&) final;

    OpMSeq1MarshaledResult opMSeq1(const Ice::Current&) final;

    OpMSeq2MarshaledResult opMSeq2(Test::StringS, const Ice::Current&) final;

    OpMDict1MarshaledResult opMDict1(const Ice::Current&) final;

    OpMDict2MarshaledResult opMDict2(Test::StringStringD, const Ice::Current&) final;

private:
    std::mutex _mutex;
    int _opByteSOnewayCallCount{0};
};

#endif
