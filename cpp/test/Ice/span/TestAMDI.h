// Copyright (c) ZeroC, Inc.

#ifndef TEST_AMD_I_H
#define TEST_AMD_I_H

#include "Test.h"

class TestIntfAMDI final : public Test::AsyncTestIntf
{
public:
    void opByteSpanAsync(
        Test::ByteSeq dataIn,
        std::function<void(std::span<const std::byte> returnValue, std::span<const std::byte> dataOut)> response,
        std::function<void(std::exception_ptr)> exception,
        const Ice::Current& current) final;

    void opShortSpanAsync(
        Test::ShortSeq dataIn,
        std::function<void(std::span<const std::int16_t> returnValue, std::span<const std::int16_t> dataOut)> response,
        std::function<void(std::exception_ptr)> exception,
        const Ice::Current& current) final;

    void opStringSpanAsync(
        Test::StringSeq dataIn,
        std::function<void(std::span<const std::string_view> returnValue, std::span<const std::string_view> dataOut)>
            response,
        std::function<void(std::exception_ptr)> exception,
        const Ice::Current& current) final;

    void opOptionalByteSpanAsync(
        std::optional<Test::ByteSeq> dataIn,
        std::function<void(
            std::optional<std::span<const std::byte>> returnValue,
            std::optional<std::span<const std::byte>> dataOut)> response,
        std::function<void(std::exception_ptr)> exception,
        const Ice::Current& current) final;

    void opOptionalShortSpanAsync(
        std::optional<Test::ShortSeq> dataIn,
        std::function<void(
            std::optional<std::span<const std::int16_t>> returnValue,
            std::optional<std::span<const std::int16_t>> dataOut)> response,
        std::function<void(std::exception_ptr)> exception,
        const Ice::Current& current) final;

    void opOptionalStringSpanAsync(
        std::optional<Test::StringSeq> dataIn,
        std::function<void(
            std::optional<std::span<const std::string_view>> returnValue,
            std::optional<std::span<const std::string_view>> dataOut)> response,
        std::function<void(std::exception_ptr)> exception,
        const Ice::Current& current) final;

    void shutdownAsync(
        std::function<void()> response,
        std::function<void(std::exception_ptr)> exception,
        const Ice::Current& current) final;
};

#endif
