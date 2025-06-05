// Copyright (c) ZeroC, Inc.

#pragma once

// No need to include <span>, since StreamHelpers.h includes it.

module Test
{
    sequence<byte> ByteSeq;
    sequence<short> ShortSeq;
    sequence<string> StringSeq;

    // No metadata on in-parameters and we don't use this generated code for the client.

    ["amd"]
    interface TestIntf
    {
        ["cpp:view-type:std::span<const std::byte>"] ByteSeq opByteSpan(
            ByteSeq dataIn,
            ["cpp:view-type:std::span<const std::byte>"] out ByteSeq dataOut);

        ["cpp:view-type:std::span<const std::int16_t>"] ShortSeq opShortSpan(
            ShortSeq dataIn,
            ["cpp:view-type:std::span<const std::int16_t>"] out ShortSeq dataOut);

        ["cpp:view-type:std::span<const std::string_view>"] StringSeq opStringSpan(
            StringSeq dataIn,
            ["cpp:view-type:std::span<const std::string_view>"] out StringSeq dataOut);

        // Same with optionals

        ["cpp:view-type:std::span<const std::byte>"] optional(10) ByteSeq opOptionalByteSpan(
            optional(1) ByteSeq dataIn,
            ["cpp:view-type:std::span<const std::byte>"] out optional(11) ByteSeq dataOut);

        ["cpp:view-type:std::span<const std::int16_t>"] optional(10) ShortSeq opOptionalShortSpan(
            optional(1) ShortSeq dataIn,
            ["cpp:view-type:std::span<const std::int16_t>"] out optional(11) ShortSeq dataOut);

        ["cpp:view-type:std::span<const std::string_view>"] optional(10) StringSeq opOptionalStringSpan(
            optional(1) StringSeq dataIn,
            ["cpp:view-type:std::span<const std::string_view>"] out optional(11) StringSeq dataOut);

        void shutdown();
    }
}
