// Copyright (c) ZeroC, Inc.
    
#pragma once
    
// No need to include <span>, since StreamHelpers.h includes it.
    
module Test
{
    sequence<byte> ByteSeq;
    sequence<short> ShortSeq;
    sequence<string> StringSeq;
        
    ["amd"]
    interface TestIntf
    {
        ["cpp:view-type:std::span<const std::byte>"] ByteSeq opByteSpan(
            ["cpp:view-type:std::span<const std::byte>"] ByteSeq dataIn,
            out ["cpp:view-type:std::span<const std::byte>"] ByteSeq dataOut);
            
        ["cpp:view-type:std::span<const std::int16_t>"] ShortSeq opShortSpan(
            ["cpp:view-type:std::span<const std::int16_t>"] ShortSeq dataIn,
            out ["cpp:view-type:std::span<const std::int16_t>"] ShortSeq dataOut);
            
        ["cpp:view-type:std::span<std::string>"] StringSeq opStringSpan(
            ["cpp:view-type:std::span<std::string>"] StringSeq dataIn,
            out ["cpp:view-type:std::span<std::string>"] StringSeq dataOut);
            
        // Same with optionals
            
        ["cpp:view-type:std::span<const std::byte>"] optional(10) ByteSeq opOptionalByteSpan(
            ["cpp:view-type:std::span<const std::byte>"] optional(1) ByteSeq dataIn,
            out ["cpp:view-type:std::span<const std::byte>"] optional(11) ByteSeq dataOut);
            
        ["cpp:view-type:std::span<const std::int16_t>"] optional(10) ShortSeq opOptionalShortSpan(
            ["cpp:view-type:std::span<const std::int16_t>"] optional(1) ShortSeq dataIn,
            out ["cpp:view-type:std::span<const std::int16_t>"] optional(11) ShortSeq dataOut);
            
        ["cpp:view-type:std::span<std::string>"] optional(10) StringSeq opOptionalStringSpan(
            ["cpp:view-type:std::span<std::string>"] optional(1) StringSeq dataIn,
            out ["cpp:view-type:std::span<std::string>"] optional(11) StringSeq dataOut);
            
        void shutdown();
    }
}
