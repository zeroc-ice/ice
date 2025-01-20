// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestIntfI final : public Test::TestIntf
{
public:
    Test::ByteSeq opByteSpan(Test::ByteSeq dataIn, Test::ByteSeq& dataOut, const Ice::Current&) final;

    Test::ShortSeq opShortSpan(Test::ShortSeq dataIn, Test::ShortSeq& dataOut, const Ice::Current&) final;

    Test::StringSeq opStringSpan(Test::StringSeq dataIn, Test::StringSeq& dataOut, const Ice::Current&) final;

    std::optional<Test::ByteSeq> opOptionalByteSpan(
        std::optional<Test::ByteSeq> dataIn,
        std::optional<Test::ByteSeq>& dataOut,
        const Ice::Current&) final;

    std::optional<Test::ShortSeq> opOptionalShortSpan(
        std::optional<Test::ShortSeq> dataIn,
        std::optional<Test::ShortSeq>& dataOut,
        const Ice::Current&) final;

    std::optional<Test::StringSeq> opOptionalStringSpan(
        std::optional<Test::StringSeq> dataIn,
        std::optional<Test::StringSeq>& dataOut,
        const Ice::Current&) final;

    void shutdown(const Ice::Current&) final;
};

#endif
