// Copyright (c) ZeroC, Inc.

#pragma once

// No need to include <span>, since StreamHelpers.h includes it.

module Test
{
    sequence<byte> ByteSeq;
    sequence<short> ShortSeq;
    sequence<string> StringSeq;

    interface TestIntf
    {
        ByteSeq opByteSpan(
            ["cpp:view-type:std::span<const std::byte>"] ByteSeq dataIn,
            out ByteSeq dataOut);

        ShortSeq opShortSpan(
            ["cpp:view-type:std::span<const std::int16_t>"] ShortSeq dataIn,
            out ShortSeq dataOut);

        StringSeq opStringSpan(
            ["cpp:view-type:std::span<const std::string_view>"] StringSeq dataIn,
            out StringSeq dataOut);

        // Same with optionals

        optional(10) ByteSeq opOptionalByteSpan(
            ["cpp:view-type:std::span<const std::byte>"] optional(1) ByteSeq dataIn,
            out optional(11) ByteSeq dataOut);

        optional(10) ShortSeq opOptionalShortSpan(
            ["cpp:view-type:std::span<const std::int16_t>"] optional(1) ShortSeq dataIn,
            out optional(11) ShortSeq dataOut);

        optional(10) StringSeq opOptionalStringSpan(
            ["cpp:view-type:std::span<const std::string_view>"] optional(1) StringSeq dataIn,
            out optional(11) StringSeq dataOut);

        void shutdown();
    }
}
