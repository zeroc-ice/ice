// Copyright (c) ZeroC, Inc.

#ifndef TEST_AMD_I_H
#define TEST_AMD_I_H

#include "Test.h"

class MyDerivedClassI final : public Test::AsyncMyDerivedClass
{
public:
    MyDerivedClassI();

    void shutdownAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) final;

    void supportsCompressAsync(std::function<void(bool)>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        final;

    void opVoidAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) final;

    void opByteAsync(
        std::uint8_t,
        std::uint8_t,
        std::function<void(std::uint8_t, std::uint8_t)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opBoolAsync(
        bool,
        bool,
        std::function<void(bool, bool)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opShortIntLongAsync(
        short,
        int,
        std::int64_t,
        std::function<void(std::int64_t, short, int, std::int64_t)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opFloatDoubleAsync(
        float,
        double,
        std::function<void(double, float, double)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opStringAsync(
        std::string,
        std::string,
        std::function<void(std::string_view, std::string_view)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMyEnumAsync(
        Test::MyEnum,
        std::function<void(Test::MyEnum, Test::MyEnum)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMyClassAsync(
        std::optional<Test::MyClassPrx>,
        std::function<void(
            const std::optional<Test::MyClassPrx>&,
            const std::optional<Test::MyClassPrx>&,
            const std::optional<Test::MyClassPrx>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opStructAsync(
        Test::Structure,
        Test::Structure,
        std::function<void(const Test::Structure&, const Test::Structure&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opByteSAsync(
        Test::ByteS,
        Test::ByteS,
        std::function<void(const Test::ByteS&, const Test::ByteS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opBoolSAsync(
        Test::BoolS,
        Test::BoolS,
        std::function<void(const Test::BoolS&, const Test::BoolS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opShortIntLongSAsync(
        Test::ShortS,
        Test::IntS,
        Test::LongS,
        std::function<void(const Test::LongS&, const Test::ShortS&, const Test::IntS&, const Test::LongS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opFloatDoubleSAsync(
        Test::FloatS,
        Test::DoubleS,
        std::function<void(const Test::DoubleS&, const Test::FloatS&, const Test::DoubleS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opStringSAsync(
        Test::StringS,
        Test::StringS,
        std::function<void(const Test::StringS&, const Test::StringS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opByteSSAsync(
        Test::ByteSS,
        Test::ByteSS,
        std::function<void(const Test::ByteSS&, const Test::ByteSS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opBoolSSAsync(
        Test::BoolSS,
        Test::BoolSS,
        std::function<void(const Test::BoolSS&, const Test::BoolSS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opShortIntLongSSAsync(
        Test::ShortSS,
        Test::IntSS,
        Test::LongSS,
        std::function<void(const Test::LongSS&, const Test::ShortSS&, const Test::IntSS&, const Test::LongSS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opFloatDoubleSSAsync(
        Test::FloatSS,
        Test::DoubleSS,
        std::function<void(const Test::DoubleSS&, const Test::FloatSS&, const Test::DoubleSS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opStringSSAsync(
        Test::StringSS,
        Test::StringSS,
        std::function<void(const Test::StringSS&, const Test::StringSS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opStringSSSAsync(
        Test::StringSSS,
        Test::StringSSS,
        std::function<void(const Test::StringSSS&, const Test::StringSSS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opByteBoolDAsync(
        Test::ByteBoolD,
        Test::ByteBoolD,
        std::function<void(const Test::ByteBoolD&, const Test::ByteBoolD&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opShortIntDAsync(
        Test::ShortIntD,
        Test::ShortIntD,
        std::function<void(const Test::ShortIntD&, const Test::ShortIntD&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opLongFloatDAsync(
        Test::LongFloatD,
        Test::LongFloatD,
        std::function<void(const Test::LongFloatD&, const Test::LongFloatD&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opStringStringDAsync(
        Test::StringStringD,
        Test::StringStringD,
        std::function<void(const Test::StringStringD&, const Test::StringStringD&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opStringMyEnumDAsync(
        Test::StringMyEnumD,
        Test::StringMyEnumD,
        std::function<void(const Test::StringMyEnumD&, const Test::StringMyEnumD&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMyEnumStringDAsync(
        Test::MyEnumStringD,
        Test::MyEnumStringD,
        std::function<void(const Test::MyEnumStringD&, const Test::MyEnumStringD&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMyStructMyEnumDAsync(
        Test::MyStructMyEnumD,
        Test::MyStructMyEnumD,
        std::function<void(const Test::MyStructMyEnumD&, const Test::MyStructMyEnumD&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opByteBoolDSAsync(
        Test::ByteBoolDS,
        Test::ByteBoolDS,
        std::function<void(const Test::ByteBoolDS&, const Test::ByteBoolDS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opShortIntDSAsync(
        Test::ShortIntDS,
        Test::ShortIntDS,
        std::function<void(const Test::ShortIntDS&, const Test::ShortIntDS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opLongFloatDSAsync(
        Test::LongFloatDS,
        Test::LongFloatDS,
        std::function<void(const Test::LongFloatDS&, const Test::LongFloatDS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opStringStringDSAsync(
        Test::StringStringDS,
        Test::StringStringDS,
        std::function<void(const Test::StringStringDS&, const Test::StringStringDS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opStringMyEnumDSAsync(
        Test::StringMyEnumDS,
        Test::StringMyEnumDS,
        std::function<void(const Test::StringMyEnumDS&, const Test::StringMyEnumDS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMyEnumStringDSAsync(
        Test::MyEnumStringDS,
        Test::MyEnumStringDS,
        std::function<void(const Test::MyEnumStringDS&, const Test::MyEnumStringDS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMyStructMyEnumDSAsync(
        Test::MyStructMyEnumDS,
        Test::MyStructMyEnumDS,
        std::function<void(const Test::MyStructMyEnumDS&, const Test::MyStructMyEnumDS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opByteByteSDAsync(
        Test::ByteByteSD,
        Test::ByteByteSD,
        std::function<void(const Test::ByteByteSD&, const Test::ByteByteSD&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opBoolBoolSDAsync(
        Test::BoolBoolSD,
        Test::BoolBoolSD,
        std::function<void(const Test::BoolBoolSD&, const Test::BoolBoolSD&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opShortShortSDAsync(
        Test::ShortShortSD,
        Test::ShortShortSD,
        std::function<void(const Test::ShortShortSD&, const Test::ShortShortSD&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opIntIntSDAsync(
        Test::IntIntSD,
        Test::IntIntSD,
        std::function<void(const Test::IntIntSD&, const Test::IntIntSD&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opLongLongSDAsync(
        Test::LongLongSD,
        Test::LongLongSD,
        std::function<void(const Test::LongLongSD&, const Test::LongLongSD&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opStringFloatSDAsync(
        Test::StringFloatSD,
        Test::StringFloatSD,
        std::function<void(const Test::StringFloatSD&, const Test::StringFloatSD&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opStringDoubleSDAsync(
        Test::StringDoubleSD,
        Test::StringDoubleSD,
        std::function<void(const Test::StringDoubleSD&, const Test::StringDoubleSD&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opStringStringSDAsync(
        Test::StringStringSD,
        Test::StringStringSD,
        std::function<void(const Test::StringStringSD&, const Test::StringStringSD&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMyEnumMyEnumSDAsync(
        Test::MyEnumMyEnumSD,
        Test::MyEnumMyEnumSD,
        std::function<void(const Test::MyEnumMyEnumSD&, const Test::MyEnumMyEnumSD&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opIntSAsync(
        Test::IntS,
        std::function<void(const Test::IntS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void
    opByteSOnewayAsync(Test::ByteS, std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        final;

    void opByteSOnewayCallCountAsync(
        std::function<void(int)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opContextAsync(
        std::function<void(const Ice::Context&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opDoubleMarshalingAsync(
        double,
        Test::DoubleS,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opIdempotentAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) final;

    void opDerivedAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) final;

    void opByte1Async(
        std::uint8_t,
        std::function<void(std::uint8_t)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opShort1Async(
        std::int16_t,
        std::function<void(std::int16_t)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opInt1Async(
        std::int32_t,
        std::function<void(std::int32_t)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opLong1Async(
        std::int64_t,
        std::function<void(std::int64_t)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opFloat1Async(float, std::function<void(float)>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        final;

    void
    opDouble1Async(double, std::function<void(double)>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        final;

    void opString1Async(
        std::string,
        std::function<void(std::string_view)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opStringS1Async(
        Test::StringS,
        std::function<void(const Test::StringS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opByteBoolD1Async(
        Test::ByteBoolD,
        std::function<void(const Test::ByteBoolD&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opStringS2Async(
        Test::StringS,
        std::function<void(const Test::StringS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opByteBoolD2Async(
        Test::ByteBoolD,
        std::function<void(const Test::ByteBoolD&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMyStruct1Async(
        Test::MyStruct1,
        std::function<void(const Test::MyStruct1&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMyClass1Async(
        std::shared_ptr<Test::MyClass1>,
        std::function<void(const std::shared_ptr<Test::MyClass1>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opStringLiteralsAsync(
        std::function<void(const Test::StringS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opWStringLiteralsAsync(
        std::function<void(const Test::WStringS&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMStruct1Async(
        std::function<void(OpMStruct1MarshaledResult)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMStruct2Async(
        Test::Structure,
        std::function<void(OpMStruct2MarshaledResult)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMSeq1Async(
        std::function<void(OpMSeq1MarshaledResult)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMSeq2Async(
        Test::StringS,
        std::function<void(OpMSeq2MarshaledResult)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMDict1Async(
        std::function<void(OpMDict1MarshaledResult)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMDict2Async(
        Test::StringStringD,
        std::function<void(OpMDict2MarshaledResult)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

private:
    std::thread _opVoidThread;
    std::mutex _opVoidMutex;

    std::mutex _mutex;
    int _opByteSOnewayCallCount{0};
};

#endif
