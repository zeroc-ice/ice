// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "TestAMD.h"

class InitialI final : public Test::Initial
{
public:
    InitialI();

    void shutdownAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) final;

    void pingPongAsync(
        Ice::ValuePtr,
        std::function<void(const Ice::ValuePtr&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opOptionalExceptionAsync(
        std::optional<std::int32_t>,
        std::optional<std::string>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opDerivedExceptionAsync(
        std::optional<std::int32_t>,
        std::optional<std::string>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opRequiredExceptionAsync(
        std::optional<std::int32_t>,
        std::optional<std::string>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opByteAsync(
        std::optional<std::uint8_t>,
        std::function<void(std::optional<std::uint8_t>, std::optional<std::uint8_t>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opBoolAsync(
        std::optional<bool>,
        std::function<void(std::optional<bool>, std::optional<bool>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opShortAsync(
        std::optional<std::int16_t>,
        std::function<void(std::optional<std::int16_t>, std::optional<std::int16_t>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opIntAsync(
        std::optional<std::int32_t>,
        std::function<void(std::optional<std::int32_t>, std::optional<std::int32_t>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opLongAsync(
        std::optional<std::int64_t>,
        std::function<void(std::optional<std::int64_t>, std::optional<std::int64_t>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opFloatAsync(
        std::optional<float>,
        std::function<void(std::optional<float>, std::optional<float>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opDoubleAsync(
        std::optional<double>,
        std::function<void(std::optional<double>, std::optional<double>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opStringAsync(
        std::optional<std::string>,
        std::function<void(std::optional<std::string_view>, std::optional<std::string_view>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMyEnumAsync(
        std::optional<Test::MyEnum>,
        std::function<void(std::optional<Test::MyEnum>, std::optional<Test::MyEnum>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opSmallStructAsync(
        std::optional<Test::SmallStruct>,
        std::function<void(const std::optional<Test::SmallStruct>&, const std::optional<Test::SmallStruct>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opFixedStructAsync(
        std::optional<Test::FixedStruct>,
        std::function<void(const std::optional<Test::FixedStruct>&, const std::optional<Test::FixedStruct>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opVarStructAsync(
        std::optional<Test::VarStruct>,
        std::function<void(const std::optional<Test::VarStruct>&, const std::optional<Test::VarStruct>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMyInterfaceProxyAsync(
        std::optional<Test::MyInterfacePrx>,
        std::function<void(const std::optional<Test::MyInterfacePrx>&, const std::optional<Test::MyInterfacePrx>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opOneOptionalAsync(
        std::shared_ptr<Test::OneOptional>,
        std::function<void(const std::shared_ptr<Test::OneOptional>&, const std::shared_ptr<Test::OneOptional>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opByteSeqAsync(
        std::optional<std::pair<const std::byte*, const std::byte*>>,
        std::function<void(
            std::optional<std::pair<const std::byte*, const std::byte*>>,
            std::optional<std::pair<const std::byte*, const std::byte*>>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opBoolSeqAsync(
        std::optional<std::pair<const bool*, const bool*>>,
        std::function<void(
            std::optional<std::pair<const bool*, const bool*>>,
            std::optional<std::pair<const bool*, const bool*>>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opShortSeqAsync(
        std::optional<std::pair<const std::int16_t*, const std::int16_t*>>,
        std::function<void(
            std::optional<std::pair<const std::int16_t*, const std::int16_t*>>,
            std::optional<std::pair<const std::int16_t*, const std::int16_t*>>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opIntSeqAsync(
        std::optional<std::pair<const std::int32_t*, const std::int32_t*>>,
        std::function<void(
            std::optional<std::pair<const std::int32_t*, const std::int32_t*>>,
            std::optional<std::pair<const std::int32_t*, const std::int32_t*>>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opLongSeqAsync(
        std::optional<std::pair<const std::int64_t*, const std::int64_t*>>,
        std::function<void(
            std::optional<std::pair<const std::int64_t*, const std::int64_t*>>,
            std::optional<std::pair<const std::int64_t*, const std::int64_t*>>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opFloatSeqAsync(
        std::optional<std::pair<const float*, const float*>>,
        std::function<void(
            std::optional<std::pair<const float*, const float*>>,
            std::optional<std::pair<const float*, const float*>>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opDoubleSeqAsync(
        std::optional<std::pair<const double*, const double*>>,
        std::function<void(
            std::optional<std::pair<const double*, const double*>>,
            std::optional<std::pair<const double*, const double*>>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opStringSeqAsync(
        std::optional<Test::StringSeq>,
        std::function<void(const std::optional<Test::StringSeq>&, const std::optional<Test::StringSeq>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opSmallStructSeqAsync(
        std::optional<std::pair<const Test::SmallStruct*, const Test::SmallStruct*>>,
        std::function<void(
            std::optional<std::pair<const Test::SmallStruct*, const Test::SmallStruct*>>,
            std::optional<std::pair<const Test::SmallStruct*, const Test::SmallStruct*>>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opSmallStructListAsync(
        std::optional<std::pair<const Test::SmallStruct*, const Test::SmallStruct*>>,
        std::function<void(
            std::optional<std::pair<const Test::SmallStruct*, const Test::SmallStruct*>>,
            std::optional<std::pair<const Test::SmallStruct*, const Test::SmallStruct*>>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opFixedStructSeqAsync(
        std::optional<std::pair<const Test::FixedStruct*, const Test::FixedStruct*>>,
        std::function<void(
            std::optional<std::pair<const Test::FixedStruct*, const Test::FixedStruct*>>,
            std::optional<std::pair<const Test::FixedStruct*, const Test::FixedStruct*>>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opFixedStructListAsync(
        std::optional<std::pair<const Test::FixedStruct*, const Test::FixedStruct*>>,
        std::function<void(
            std::optional<std::pair<const Test::FixedStruct*, const Test::FixedStruct*>>,
            std::optional<std::pair<const Test::FixedStruct*, const Test::FixedStruct*>>)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opVarStructSeqAsync(
        std::optional<Test::VarStructSeq>,
        std::function<void(const std::optional<Test::VarStructSeq>&, const std::optional<Test::VarStructSeq>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opSerializableAsync(
        std::optional<Test::Serializable>,
        std::function<void(const std::optional<Test::Serializable>&, const std::optional<Test::Serializable>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opIntIntDictAsync(
        std::optional<Test::IntIntDict>,
        std::function<void(const std::optional<Test::IntIntDict>&, const std::optional<Test::IntIntDict>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opStringIntDictAsync(
        std::optional<Test::StringIntDict>,
        std::function<void(const std::optional<Test::StringIntDict>&, const std::optional<Test::StringIntDict>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opClassAndUnknownOptionalAsync(
        std::shared_ptr<Test::A>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opGAsync(
        std::shared_ptr<Test::G>,
        std::function<void(const std::shared_ptr<Test::G>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opVoidAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) final;

    void opMStruct1Async(
        std::function<void(OpMStruct1MarshaledResult)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMStruct2Async(
        std::optional<Test::SmallStruct>,
        std::function<void(OpMStruct2MarshaledResult)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMSeq1Async(
        std::function<void(OpMSeq1MarshaledResult)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMSeq2Async(
        std::optional<Test::StringSeq>,
        std::function<void(OpMSeq2MarshaledResult)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMDict1Async(
        std::function<void(OpMDict1MarshaledResult)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opMDict2Async(
        std::optional<Test::StringIntDict>,
        std::function<void(OpMDict2MarshaledResult)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void supportsJavaSerializableAsync(
        std::function<void(bool)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;
};

#endif
